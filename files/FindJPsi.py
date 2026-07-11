import argparse as ap
import ROOT
import numpy as np

from podio.reading import get_reader

# Default options
in_file_def   = "lAger3.6.1-1.0_jpsi_10x130_hiAcc_run1.0009.eicrecon.edm4eic.root"
out_file_def  = "jpsi_search_py.analysis.root"
match_cut_def = 0.2
ep_min_def    = 0.8
ep_max_def    = 1.2
mass_min_def  = 2.5
mass_max_def  = 3.5


def FindJPsi(
    in_file   = in_file_def, 
    out_file  = out_file_def,
    match_cut = match_cut_def,
    ep_min    = ep_min_def,
    ep_max    = ep_max_def,
    mass_min  = mass_min_def,
    mass_max  = mass_max_def,
):

    # Open reader and output file ============================================

    # use podio reader to open example EICrecon output
    reader = get_reader(in_file)

    # Create histograms ======================================================

    h_track_momentum_all     = ROOT.TH1D("h_track_momentum_all", "Track #it{p} (all)", 50, 0.0, 10.0)
    h_track_momentum_match   = ROOT.TH1D("h_track_momentum_match", "Track #it{p} (matched to cluster)", 50, 0.0, 10.0)
    h_track_eta_all          = ROOT.TH1D("h_track_eta_all", "Track #eta (all)", 250, -5.0, 5.0)
    h_track_eta_match        = ROOT.TH1D("h_track_eta_match", "Track #eta (matched to cluster)", 250, -5.0, 5.0)
    h_track_cluster_eop      = ROOT.TH1D("h_track_cluster_eop", "Track-cluster E/p", 40, 0.0, 2.0)
    h_invariant_mass         = ROOT.TH1D("h_invariant_mass", "Reconstructed invariant mass", 50, 0.0, 5.0)
    h_daughter_momentum_all  = ROOT.TH1D("h_daughter_momentum_all", "Daughter e^{#pm} #it{p} (all)", 50, 0.0, 10.0)
    h_daughter_momentum_reco = ROOT.TH1D("h_daughter_momentum_reco", "Daughter e^{#pm} #it{p} (reconstructed)", 50, 0.0, 10.0)
    h_daughter_eta_all       = ROOT.TH1D("h_daughter_eta_all", "Daughter e^{#pm} #eta (all)", 250, -5.0, 5.0)
    h_daughter_eta_reco      = ROOT.TH1D("h_daughter_eta_reco", "Daughter e^{#pm} #eta (reconstructed)", 250, -5.0, 5.0)
    h_jpsi_momentum_all      = ROOT.TH1D("h_jpsi_momentum_all", "J/#psi #it{p} (all e^{#pm} decays)", 50, 0.0, 10.0)
    h_jpsi_momentum_reco     = ROOT.TH1D("h_jpsi_momentum_reco", "J/#psi #it{p} (reconstructed e^{#pm} decays)", 50, 0.0, 10.0)
    h_jpsi_eta_all           = ROOT.TH1D("h_jpsi_eta_all", "J/#psi #eta (all e^{#pm} decays)", 250, -5.0, 5.0)
    h_jpsi_eta_reco          = ROOT.TH1D("h_jpsi_eta_reco", "J/#psi #eta (reconstructed e^{#pm} decays)", 250, -5.0, 5.0)

    # Event loop ==============================================================

    for frame in reader.get("events"):

        # Grab all the collections we'll need
        particles    = frame.get("MCParticles");
        tracks       = frame.get("CentralCKFTracks");
        clusters_neg = frame.get("EcalEndcapNClusters");
        clusters_cen = frame.get("EcalBarrelClusters");
        clusters_pos = frame.get("EcalEndcapPClusters");
        associations = frame.get("CentralCKFTrackAssociations");

        # Step 1: match tracks-to-cluster to find electrons ===================

        # Convert edm4hep::Vector3f into a ROOT::Math::XZYVector
        #    --> Useful for some vector calculations
        def convert_vector(edm_vec):
            return ROOT.Math.XYZVector(edm_vec.x, edm_vec.y, edm_vec.z)

        # Get cluster 3-momentum using its position and energy
        #   --> Assuming interaction vetex is at origin!
        def get_momentum(position, energy):
            unit = position.Unit()
            return ROOT.Math.XYZVector(  # * operator not implemented
                energy * unit.X(),
                energy * unit.Y(),
                energy * unit.Z(),
            )

        # Get distance in eta-phi space between 2 3-vectors
        #   --> For matching tracks and clusters
        def get_distance(vec_1, vec_2):
            return np.hypot(vec_1.Eta() - vec_2.Eta(), vec_1.Phi() - vec_2.Phi());

        # Loop over tracks ----------------------------------------------------

        rec_electrons = list()
        used_clusters = set()
        for track in tracks:

            trk_mom = convert_vector(track.getMomentum())
            trk_eta = trk_mom.Eta()
            trk_mag = np.sqrt(trk_mom.Mag2())  # Mag() isn't implemented for XYZVectors
            h_track_momentum_all.Fill(trk_mag)
            h_track_eta_all.Fill(trk_eta)

            best_distance = 999.0
            best_match    = None

            # Compare track against negative ECal -----------------------------

            for cluster in clusters_neg:

                if cluster in used_clusters:
                    continue

                clust_pos = convert_vector(cluster.getPosition())
                clust_mom = get_momentum(clust_pos, cluster.getEnergy())
                distance  = get_distance(trk_mom, clust_mom)

                if (distance < match_cut) and (distance < best_distance):
                    best_distance = distance
                    best_match    = cluster
                    used_clusters.add(cluster)

            # Compare track against central ECal ------------------------------

            for cluster in clusters_cen:

                if cluster in used_clusters:
                    continue

                clust_pos = convert_vector(cluster.getPosition())
                clust_mom = get_momentum(clust_pos, cluster.getEnergy())
                distance  = get_distance(trk_mom, clust_mom)

                if (distance < match_cut) and (distance < best_distance):
                    best_distance = distance
                    best_match    = cluster
                    used_clusters.add(cluster)

            # Compare track against positive ECal -----------------------------

            for cluster in clusters_pos:

                if cluster in used_clusters:
                    continue

                clust_pos = convert_vector(cluster.getPosition())
                clust_mom = get_momentum(clust_pos, cluster.getEnergy())
                distance  = get_distance(trk_mom, clust_mom)

                if (distance < match_cut) and (distance < best_distance):
                    best_distance = distance
                    best_match    = cluster
                    used_clusters.add(cluster)

            # Compare track vs. matched cluster, identify e- candidates -------

            if best_match is not None:
                h_track_momentum_match.Fill(trk_mag)
                h_track_eta_match.Fill(trk_eta)
            else:
                continue

            match_ene = best_match.getEnergy()
            trk_ep    = match_ene / trk_mag
            if trk_mag > 0.0:
                h_track_cluster_eop.Fill(trk_ep)
            else:
                continue
      
            if (trk_ep > ep_min) and (trk_ep < ep_max):
                rec_electrons.append(track)

        # Step 2: reconstruct J/psi ===========================================

        # Get 4-vector for a track with a certain mass
        #   --> Will need for invariant mass!
        def get_lorentz_track(track, mass):
            momentum = track.getMomentum()
            energy   = np.sqrt(mass**2 + momentum.x**2 + momentum.y**2 + momentum.z**2)
            return ROOT.Math.XYZTVector(
                momentum.x,
                momentum.y,
                momentum.z,
                energy
            )

        # Loop over pairs of electrons ----------------------------------------

        # Our J/psi candidates will be pairs of identified electrons
        #   --> So need list of pairs of tracks
        used_electrons  = set()
        jpsi_candidates = list()

        for electron_1 in rec_electrons:
            for electron_2 in rec_electrons:

                # Skip diagonal, make sure we don't double count tracks
                if electron_1 == electron_2:
                    continue

                # Make sure we don't double count tracks
                if electron_1 in used_electrons:
                    continue
                if electron_2 in used_electrons:
                    continue

                # Calculate invariant mass to find J/psi ----------------------

                lorentz_1 = get_lorentz_track(electron_1, 0.000511)
                lorentz_2 = get_lorentz_track(electron_2, 0.000511)

                total_mom = ROOT.Math.XYZTVector(
                    lorentz_1.Px() + lorentz_2.Px(),
                    lorentz_1.Py() + lorentz_2.Py(),
                    lorentz_1.Pz() + lorentz_2.Pz(),
                    lorentz_1.E() + lorentz_2.E()
                )
                inv_mass = total_mom.M();
                h_invariant_mass.Fill(inv_mass);

                if (inv_mass > mass_min) and (inv_mass < mass_max):
                    jpsi_candidates.append(
                        (electron_1, electron_2)
                    )
                    used_electrons.add(electron_1)
                    used_electrons.add(electron_2)

        # Step 3: Compare against truth info ==================================

        # Get 4-vector for a particle
        #   --> Will need to compare rec vs. sim momentum/eta 
        def get_lorentz_particle(particle):
            momentum = particle.getMomentum()
            mass     = particle.getMass()
            energy   = np.sqrt(mass**2 + momentum.x**2 + momentum.y**2 + momentum.z**2)
            return ROOT.Math.XYZTVector(
                momentum.x,
                momentum.y,
                momentum.z,
                energy,
            )

        # We want to check if we found BOTH daughters in our pair. So we need 3 maps:
        #   --> track to associated daughter
        #   --> one daughter to the other daughter
        #   --> daughters to j/psi
        track_to_daughter    = dict()
        daughter_to_daughter = dict()
        daughter_to_parent   = dict()

        for particle in particles:
            if particle.getPDG() == 443:

                # Find decay leptons ------------------------------------------

                is_electron_decay = False
                decays            = list()
                for daughter in particle.getDaughters():
                    if abs(daughter.getPDG()) == 11:

                        # If daughter is an electron/positron, then this is
                        # a J/psi --> e+e- decay
                        is_electron_decay = True
                        decays.append(daughter)

                        ele_mom = convert_vector(particle.getMomentum())
                        ele_eta = ele_mom.Eta()
                        ele_mag = np.sqrt(ele_mom.Mag2())  # Mag() isn't implemented for XYZVector
                        h_daughter_momentum_all.Fill(ele_mag)
                        h_daughter_eta_all.Fill(ele_eta)

                        rec_ele = None
                        for association in associations:
                            if association.getSim() == daughter:
                                rec_ele = association.getRec()
                                break

                        if rec_ele is not None:
                            h_daughter_momentum_reco.Fill(ele_mag)
                            h_daughter_eta_reco.Fill(ele_eta)
                            track_to_daughter[rec_ele] = daughter

                # Sanity check: should have found 2 leptons
                #   --> If so, add their object IDs to map and fill histograms
                if len(decays) == 2:
                    daughter_to_daughter[decays[0]]  = decays[-1]
                    daughter_to_daughter[decays[-1]] = decays[0]
                    daughter_to_parent[decays[0]]    = particle
                    daughter_to_parent[decays[-1]]   = particle

                jpsi_lorentz = get_lorentz_particle(particle)
                h_jpsi_momentum_all.Fill(jpsi_lorentz.P())
                h_jpsi_eta_all.Fill(jpsi_lorentz.Eta())

        # Compare reco lepton pairs to truth ones -----------------------------

        for track_1, track_2 in jpsi_candidates:

            # Check if tracks correspond to decay leptons
            par_1 = None
            if track_1 in track_to_daughter:
                par_1 = track_to_daughter[track_1]

            par_2 = None
            if track_2 in track_to_daughter:
                par_2 = track_to_daughter[track_2]

            if (par_1 is None) or (par_2 is None):
                continue

            # Now check if pair of tracks correspond to
            # a valid pair of electrons
            #   --> If they do, then we should see them
            #       match in our daughter_to_daughter
            #       map
            other_par_1 = daughter_to_daughter[par_1]
            other_par_2 = daughter_to_daughter[par_2]

            if (other_par_1 == par_2) and (other_par_2 == par_1):
                sim_jpsi     = daughter_to_parent[other_par_1]
                jpsi_lorentz = get_lorentz_particle(sim_jpsi)
                h_jpsi_momentum_reco.Fill(jpsi_lorentz.P())
                h_jpsi_eta_reco.Fill(jpsi_lorentz.Eta())

    # Calculate efficiencies and save output ==================================

    h_track_momentum_efficiency = h_track_momentum_all.Clone()
    h_track_momentum_efficiency.SetNameTitle(
        "h_track_momentum_efficiency",
        "Efficiency of matching tracks to clusters vs. track #it{p}")
    h_track_momentum_efficiency.Divide(h_track_momentum_match, h_track_momentum_all)

    h_track_eta_efficiency = h_track_eta_all.Clone()
    h_track_eta_efficiency.SetNameTitle(
        "h_track_eta_efficiency",
        "Efficiency of matching of tracks to clusters vs. track #eta")
    h_track_eta_efficiency.Divide(h_track_eta_match, h_track_eta_all)

    h_daughter_momentum_efficiency = h_daughter_momentum_all.Clone()
    h_daughter_momentum_efficiency.SetNameTitle(
        "h_daughter_momentum_efficiency",
        "Efficiency of reconstructing J/#psi daughter e^{#pm} vs. daughter #it{p}")
    h_daughter_momentum_efficiency.Divide(h_daughter_momentum_reco, h_daughter_momentum_all)

    h_daughter_eta_efficiency = h_daughter_eta_all.Clone()
    h_daughter_eta_efficiency.SetNameTitle(
        "h_daughter_eta_efficiency",
        "Efficiency of reconstructing J/#psi daughter e^{#pm} vs. daughter #eta")
    h_daughter_eta_efficiency.Divide(h_daughter_eta_reco, h_daughter_eta_all)

    h_jpsi_momentum_efficiency = h_jpsi_momentum_all.Clone()
    h_jpsi_momentum_efficiency.SetNameTitle(
        "h_jpsi_momentum_efficiency",
        "Efficiency of reconstructing J/#psi #rightarrow e^{+}+e^{-} vs. J/#psi #it{p}")
    h_jpsi_momentum_efficiency.Divide(h_jpsi_momentum_reco, h_jpsi_momentum_all)

    h_jpsi_eta_efficiency = h_jpsi_eta_all.Clone()
    h_jpsi_eta_efficiency.SetNameTitle(
        "h_jpsi_eta_efficiency",
        "Efficiency of reconstructing J/#psi #rightarrow e^{+}+e^{-} vs. J/#psi #eta")
    h_jpsi_eta_efficiency.Divide(h_jpsi_eta_reco, h_jpsi_eta_all)

    with ROOT.TFile(out_file, "recreate") as ofile:
        ofile.WriteObject(h_track_momentum_all)
        ofile.WriteObject(h_track_momentum_match)
        ofile.WriteObject(h_track_momentum_efficiency)
        ofile.WriteObject(h_track_eta_all)
        ofile.WriteObject(h_track_eta_match)
        ofile.WriteObject(h_track_eta_efficiency)
        ofile.WriteObject(h_track_cluster_eop)
        ofile.WriteObject(h_invariant_mass)
        ofile.WriteObject(h_daughter_momentum_all)
        ofile.WriteObject(h_daughter_momentum_reco)
        ofile.WriteObject(h_daughter_momentum_efficiency)
        ofile.WriteObject(h_daughter_eta_all)
        ofile.WriteObject(h_daughter_eta_reco)
        ofile.WriteObject(h_daughter_eta_efficiency)
        ofile.WriteObject(h_jpsi_momentum_all)
        ofile.WriteObject(h_jpsi_momentum_reco)
        ofile.WriteObject(h_jpsi_momentum_efficiency)
        ofile.WriteObject(h_jpsi_eta_all)
        ofile.WriteObject(h_jpsi_eta_reco)
        ofile.WriteObject(h_jpsi_eta_efficiency)


# Main ========================================================================

if __name__ == "__main__":

    # set up argments
    parser = ap.ArgumentParser()
    parser.add_argument("--infile", default = in_file_def)
    parser.add_argument("--outfile", default = out_file_def)
    parser.add_argument("--matchcut", default = match_cut_def)
    parser.add_argument("--epmin", default = ep_min_def)
    parser.add_argument("--epmax", default = ep_max_def)
    parser.add_argument("--massmin", default = mass_min_def)
    parser.add_argument("--massmax", default = mass_max_def)
    args = parser.parse_args()

    # Run calculation
    FindJPsi(
        args.infile,
        args.outfile,
        args.matchcut,
        args.epmin,
        args.epmax,
        args.massmin,
        args.massmax,
    )
