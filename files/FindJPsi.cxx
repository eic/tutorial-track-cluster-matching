#include <edm4eic/ClusterCollection.h>
#include <edm4eic/MCRecoTrackParticleAssociationCollection.h>
#include <edm4eic/TrackCollection.h>
#include <edm4hep/MCParticleCollection.h>
#include <edm4hep/Vector3f.h>
#include <podio/Frame.h>
#include <podio/ObjectID.h>
#include <podio/ROOTReader.h>
#include <Math/Vector3D.h>
#include <Math/Vector4D.h>
#include <TFile.h>
#include <TH1.h>
#include <cmath>
#include <optional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>
#include <utility>


void FindJPsi(
  const std::string& in_file   = "lAger3.6.1-1.0_jpsi_10x130_hiAcc_run1.0009.eicrecon.edm4eic.root",
  const std::string& out_file  = "jpsi_search_cxx.analysis.root",
  const float        match_cut = 0.2,
  const float        ep_min    = 0.8,
  const float        ep_max    = 1.2,
  const float        mass_min  = 2.5,
  const float        mass_max  = 3.5
) {

  // Open reader and output file ==============================================

  podio::ROOTReader reader = podio::ROOTReader();
  reader.openFile(in_file);

  TFile* output = new TFile(out_file.data(), "recreate");

  // Create histograms ========================================================

  TH1D* h_track_momentum_all     = new TH1D("h_track_momentum_all", "Track #it{p} (all)", 50, 0.0, 10.0);
  TH1D* h_track_momentum_match   = new TH1D("h_track_momentum_match", "Track #it{p} (matched to cluster)", 50, 0.0, 10.0);
  TH1D* h_track_eta_all          = new TH1D("h_track_eta_all", "Track #eta (all)", 250, -5.0, 5.0);
  TH1D* h_track_eta_match        = new TH1D("h_track_eta_match", "Track #eta (matched to cluster)", 250, -5.0, 5.0);
  TH1D* h_track_cluster_eop      = new TH1D("h_track_cluster_eop", "Track-cluster E/p", 40, 0.0, 2.0);
  TH1D* h_invariant_mass         = new TH1D("h_invariant_mass", "Reconstructed invariant mass", 50, 0.0, 5.0);
  TH1D* h_daughter_momentum_all  = new TH1D("h_daughter_momentum_all", "Daughter e^{#pm} #it{p} (all)", 50, 0.0, 10.0);
  TH1D* h_daughter_momentum_reco = new TH1D("h_daughter_momentum_reco", "Daughter e^{#pm} #it{p} (reconstructed)", 50, 0.0, 10.0);
  TH1D* h_daughter_eta_all       = new TH1D("h_daughter_eta_all", "Daughter e^{#pm} #eta (all)", 250, -5.0, 5.0);
  TH1D* h_daughter_eta_reco      = new TH1D("h_daughter_eta_reco", "Daughter e^{#pm} #eta (reconstructed)", 250, -5.0, 5.0);
  TH1D* h_jpsi_momentum_all      = new TH1D("h_jpsi_momentum_all", "J/#psi #it{p} (all e^{#pm} decays)", 50, 0.0, 10.0);
  TH1D* h_jpsi_momentum_reco     = new TH1D("h_jpsi_momentum_reco", "J/#psi #it{p} (reconstructed e^{#pm} decays)", 50, 0.0, 10.0);
  TH1D* h_jpsi_eta_all           = new TH1D("h_jpsi_eta_all", "J/#psi #eta (all e^{#pm} decays)", 250, -5.0, 5.0);
  TH1D* h_jpsi_eta_reco          = new TH1D("h_jpsi_eta_reco", "J/#psi #eta (reconstructed e^{#pm} decays)", 250, -5.0, 5.0);

  // Event loop ===============================================================

  while (auto entry = reader.readNextEntry(podio::Category::Event)) {

    auto frame = podio::Frame(std::move(entry));

    // Grab all the collections we'll need
    auto& particles    = frame.get<edm4hep::MCParticleCollection>("MCParticles");
    auto& tracks       = frame.get<edm4eic::TrackCollection>("CentralCKFTracks");
    auto& clusters_neg = frame.get<edm4eic::ClusterCollection>("EcalEndcapNClusters");
    auto& clusters_cen = frame.get<edm4eic::ClusterCollection>("EcalBarrelClusters");
    auto& clusters_pos = frame.get<edm4eic::ClusterCollection>("EcalEndcapPClusters");
    auto& associations = frame.get<edm4eic::MCRecoTrackParticleAssociationCollection>(
      "CentralCKFTrackAssociations");

    // Step 1: match tracks-to-cluster to find electrons ======================

    // Convert edm4hep::Vector3f into a ROOT::Math::XZYVector
    //   --> Useful for some vector calculations
    auto convert_vector_float = [](const edm4hep::Vector3f& edm_vec) {
      return ROOT::Math::XYZVector(edm_vec.x, edm_vec.y, edm_vec.z);
    };

    // Get cluster 3-momentum using its position and energy
    //   --> Assuming interaction vetex is at origin!
    auto get_momentum = [](const ROOT::Math::XYZVector& position, const double energy) {
      return energy * position.Unit();
    };

    // Get distance in eta-phi space between 2 3-vectors
    //   --> For matching tracks and clusters
    auto get_distance = [](const ROOT::Math::XYZVector& vec_1, const ROOT::Math::XYZVector& vec_2) {
      return std::hypot(vec_1.Eta() - vec_2.Eta(), vec_1.Phi() - vec_2.Phi());
    };

    // Compare 2 object IDs
    //   --> Needed for bookkeeping
    auto compare_object_ids = [](const podio::ObjectID& lhs, const podio::ObjectID& rhs) {
      if (lhs.collectionID == rhs.collectionID) {
        return (lhs.index < rhs.index);
      } else {
        return (lhs.collectionID < rhs.collectionID);
      }
    };

    // Loop over tracks -------------------------------------------------------

    std::vector<edm4eic::Track> rec_electrons;
    std::set<podio::ObjectID, decltype(compare_object_ids)> used_clusters;

    for (const auto& track : tracks) {

      const auto  trk_mom = convert_vector_float(track.getMomentum());
      const float trk_eta = trk_mom.Eta();
      const float trk_mag = std::sqrt(trk_mom.Mag2());  // Mag() isn't implemented for XYZVectors
      h_track_momentum_all->Fill(trk_mag);
      h_track_eta_all->Fill(trk_eta);

      float best_distance = 999.0;
      std::optional<edm4eic::Cluster> best_match;

      // Compare track against negative ECal ----------------------------------

      for (const auto& cluster : clusters_neg) {

        if (used_clusters.contains(cluster.getObjectID())) {
          continue;
        }

        const auto  clust_pos = convert_vector_float(cluster.getPosition());
        const auto  clust_mom = get_momentum(clust_pos, cluster.getEnergy());
        const float distance  = get_distance(trk_mom, clust_mom);

        if ((distance < match_cut) && (distance < best_distance)) {
          best_distance = distance;
          best_match    = cluster;
          used_clusters.insert(cluster.getObjectID());
        }
      }

      // Compare track against central ECal -----------------------------------

      for (const auto& cluster : clusters_cen) {

        if (used_clusters.contains(cluster.getObjectID())) {
          continue;
        }

        const auto  clust_pos = convert_vector_float(cluster.getPosition());
        const auto  clust_mom = get_momentum(clust_pos, cluster.getEnergy());
        const float distance = get_distance(trk_mom, clust_mom);

        if ((distance < match_cut) && (distance < best_distance)) {
          best_distance = distance;
          best_match    = cluster;
          used_clusters.insert(cluster.getObjectID());
        }
      }

      // Compare track against positive ECal ----------------------------------

      for (const auto& cluster : clusters_pos) {

        if (used_clusters.contains(cluster.getObjectID())) {
          continue;
        }

        const auto  clust_pos = convert_vector_float(cluster.getPosition());
        const auto  clust_mom = get_momentum(clust_pos, cluster.getEnergy());
        const float distance = get_distance(trk_mom, clust_mom);

        if ((distance < match_cut) && (distance < best_distance)) {
          best_distance = distance;
          best_match    = cluster;
          used_clusters.insert(cluster.getObjectID());
        }
      }

      // Compare track vs. matched cluster, identify e- candidates ------------

      if (best_match.has_value()) {
        h_track_momentum_match->Fill(trk_mag);
        h_track_eta_match->Fill(trk_eta);
      } else {
        continue;
      }

      const float match_ene = best_match.value().getEnergy();
      const float trk_ep    = match_ene / trk_mag;
      if (trk_mag > 0.0) {
        h_track_cluster_eop->Fill(trk_ep);
      } else {
        continue;
      }
      
      if ((trk_ep > ep_min) && (trk_ep < ep_max)) {
        rec_electrons.push_back(track);
      }
    }  // end track loop

    // Step 2: reconstruct J/psi ==============================================

    // Get 4-vector for a track with a certain mass
    //   --> Will need for invariant mass!
    auto get_lorentz_track = [](const edm4eic::Track& track, const float mass) {
      return ROOT::Math::PxPyPzM4D(
        track.getMomentum().x,
        track.getMomentum().y,
        track.getMomentum().z,
        mass
      );
    };

    // Loop over pairs of electrons ------------------------------------------

    // Our J/psi candidates will be pairs of identified electrons
    //   --> So need vector of pairs of tracks
    std::set<podio::ObjectID, decltype(compare_object_ids)> used_electrons;
    std::vector<std::pair<edm4eic::Track, edm4eic::Track>> jpsi_candidates;

    for (const auto& electron_1 : rec_electrons) {
      for (const auto& electron_2 : rec_electrons) {

         // Skip diagonal, make sure we don't double count tracks
         if (electron_1.getObjectID() == electron_2.getObjectID()) {
           continue;
         }
         if (used_electrons.contains(electron_1.getObjectID())) {
           continue;
         }
         if (used_electrons.contains(electron_2.getObjectID())) {
           continue;
         }

         // Calculate invariant mass to find J/psi ----------------------------

         const auto lorentz_1 = get_lorentz_track(electron_1, 0.000511);
         const auto lorentz_2 = get_lorentz_track(electron_2, 0.000511);

         const ROOT::Math::PxPyPzE4D total_mom(
           lorentz_1.Px() + lorentz_2.Px(),
           lorentz_1.Py() + lorentz_2.Py(),
           lorentz_1.Pz() + lorentz_2.Pz(),
           lorentz_1.E() + lorentz_2.E()
         );
         const float inv_mass = total_mom.M();
         h_invariant_mass->Fill(inv_mass);

         if ((inv_mass > mass_min) && (inv_mass < mass_max)) {
           jpsi_candidates.push_back(
            {electron_1, electron_2}
           );
           used_electrons.insert(electron_1.getObjectID());
           used_electrons.insert(electron_2.getObjectID());
         }
       }
    }

    // Step 3: Compare against truth info =====================================

    // Convert edm4hep::Vector3d into a ROOT::Math::XZYVector
    //   --> edm4hep stores momentum values as doubles, not floats
    auto convert_vector_double = [](const edm4hep::Vector3d& edm_vec) {
      return ROOT::Math::XYZVector(edm_vec.x, edm_vec.y, edm_vec.z);
    };

    // Get 4-vector for a particle
    //   --> Will need to compare rec vs. sim momentum/eta 
    auto get_lorentz_particle = [](const edm4hep::MCParticle& particle) {
      return ROOT::Math::PxPyPzM4D(
        particle.getMomentum().x,
        particle.getMomentum().y,
        particle.getMomentum().z,
        particle.getMass()
      );
    };

    // We want to check if we found BOTH daughters in our pair. So we need 3 maps:
    //   --> track to associated daughter
    //   --> one daughter to the other daughter
    //   --> daughters to j/psi
    std::map<podio::ObjectID, podio::ObjectID, decltype(compare_object_ids)> track_to_daughter;
    std::map<podio::ObjectID, podio::ObjectID, decltype(compare_object_ids)> daughter_to_daughter;
    std::map<podio::ObjectID, edm4hep::MCParticle, decltype(compare_object_ids)> daughter_to_parent;

    for (const auto& particle : particles) {
      if (particle.getPDG() == 443) {

        // Find decay leptons -------------------------------------------------

        bool is_electron_decay = false;
        std::vector<edm4hep::MCParticle> decays;

        for (const auto& daughter : particle.getDaughters()) {
          if (std::abs(daughter.getPDG()) == 11) {

            // If daughter is an electron/positron, then this is
            // a J/psi --> e+e- decay
            is_electron_decay = true;
            decays.push_back(daughter);

            const auto  ele_mom = convert_vector_double(particle.getMomentum());
            const float ele_eta = ele_mom.Eta();
            // Mag() isn't implemented for XYZVector
            const float ele_mag = std::sqrt(ele_mom.Mag2());
            h_daughter_momentum_all->Fill(ele_mag);
            h_daughter_eta_all->Fill(ele_eta);

            std::optional<edm4eic::Track> rec_ele;
            for (const auto& association : associations) {
              if (association.getSim() == daughter) {
                rec_ele = association.getRec();
                break;
              }
            }

            if (rec_ele.has_value()) {
              h_daughter_momentum_reco->Fill(ele_mag);
              h_daughter_eta_reco->Fill(ele_eta);
              track_to_daughter[rec_ele.value().getObjectID()] = daughter.getObjectID();
            }
          }
        }  // end daughter loop

        // Sanity check: should have found 2 leptons
        //   --> If so, add their object IDs to map and fill histograms
        if (decays.size() == 2) {
          daughter_to_daughter[decays.front().getObjectID()] = decays.back().getObjectID();
          daughter_to_daughter[decays.back().getObjectID()]  = decays.front().getObjectID();
          daughter_to_parent[decays.front().getObjectID()]   = particle;
          daughter_to_parent[decays.back().getObjectID()]    = particle;

          const auto jpsi_lorentz = get_lorentz_particle(particle);
          h_jpsi_momentum_all->Fill(jpsi_lorentz.P());
          h_jpsi_eta_all->Fill(jpsi_lorentz.Eta());
        }
      }
    }  // end particle loop

    // Compare reco lepton pairs to truth ones --------------------------------

    for (const auto& [track_1, track_2] : jpsi_candidates) {

      // Check if tracks correspond to decay leptons
      std::optional<podio::ObjectID> par_id_1;
      if (track_to_daughter.count(track_1.getObjectID()) > 0) {
        par_id_1 = track_to_daughter[track_1.getObjectID()];
      }

      std::optional<podio::ObjectID> par_id_2;
      if (track_to_daughter.count(track_2.getObjectID()) > 0) {
        par_id_2 = track_to_daughter[track_2.getObjectID()];
      }

      if (!par_id_1.has_value() || !par_id_2.has_value()) {
        continue;
      }

      // Now check if pair of tracks correspond to
      // a valid pair of electrons
      //   --> If they do, then we should see their
      //       Object IDs match in our daughter_to_daughter
      //       map
      const auto other_par_id_1 = daughter_to_daughter[par_id_1.value()];
      const auto other_par_id_2 = daughter_to_daughter[par_id_2.value()];

      if ((other_par_id_1 == par_id_2.value()) && (other_par_id_2 == par_id_1.value())) {
        const auto& sim_jpsi     = daughter_to_parent[other_par_id_1];
        const auto  jpsi_lorentz = get_lorentz_particle(sim_jpsi);
        h_jpsi_momentum_reco->Fill(jpsi_lorentz.P());
        h_jpsi_eta_reco->Fill(jpsi_lorentz.Eta());
      }
    }
  }  // end event loop

  // Calculate efficiencies and save output ===================================

  TH1D* h_track_momentum_efficiency = (TH1D*) h_track_momentum_all->Clone();
  h_track_momentum_efficiency->SetNameTitle(
    "h_track_momentum_efficiency",
    "Efficiency of matching tracks to clusters vs. track #it{p}");
  h_track_momentum_efficiency->Divide(h_track_momentum_match, h_track_momentum_all);

  TH1D* h_track_eta_efficiency = (TH1D*) h_track_eta_all->Clone();
  h_track_eta_efficiency->SetNameTitle(
    "h_track_eta_efficiency",
    "Efficiency of matching of tracks to clusters vs. track #eta");
  h_track_eta_efficiency->Divide(h_track_eta_match, h_track_eta_all);

  TH1D* h_daughter_momentum_efficiency = (TH1D*) h_daughter_momentum_all->Clone();
  h_daughter_momentum_efficiency->SetNameTitle(
    "h_daughter_momentum_efficiency",
    "Efficiency of reconstructing J/#psi daughter e^{#pm} vs. daughter #it{p}");
  h_daughter_momentum_efficiency->Divide(h_daughter_momentum_reco, h_daughter_momentum_all);

  TH1D* h_daughter_eta_efficiency = (TH1D*) h_daughter_eta_all->Clone();
  h_daughter_eta_efficiency->SetNameTitle(
    "h_daughter_eta_efficiency",
    "Efficiency of reconstructing J/#psi daughter e^{#pm} vs. daughter #eta");
  h_daughter_eta_efficiency->Divide(h_daughter_eta_reco, h_daughter_eta_all);

  TH1D* h_jpsi_momentum_efficiency = (TH1D*) h_jpsi_momentum_all->Clone();
  h_jpsi_momentum_efficiency->SetNameTitle(
    "h_jpsi_momentum_efficiency",
    "Efficiency of reconstructing J/#psi #rightarrow e^{+}+e^{-} vs. J/#psi #it{p}");
  h_jpsi_momentum_efficiency->Divide(h_jpsi_momentum_reco, h_jpsi_momentum_all);

  TH1D* h_jpsi_eta_efficiency = (TH1D*) h_jpsi_eta_all->Clone();
  h_jpsi_eta_efficiency->SetNameTitle(
    "h_jpsi_eta_efficiency",
    "Efficiency of reconstructing J/#psi #rightarrow e^{+}+e^{-} vs. J/#psi #eta");
  h_jpsi_eta_efficiency->Divide(h_jpsi_eta_reco, h_jpsi_eta_all);

  output->cd();
  output->Write();
  output->Close();
  return;

}
