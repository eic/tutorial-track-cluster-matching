---
title: "Track-Cluster Matching"
teaching: 10
exercises: 35
---

::::::::::::::::::::::::::::::::::::::::::::: questions

- How do actually I use PODIO?

:::::::::::::::::::::::::::::::::::::::::::::

::::::::::::::::::::::::::::::::::::::::::::: objectives

- Use PODIO interface to find reconstructed J/psi

:::::::::::::::::::::::::::::::::::::::::::::

## Exercise Goal

Now that you have some background, let's get some hands-on experience with
PODIO.  Our goal in this epsiode will be to see if we can reconstruct $J/\psi$ .
The next episode will then have us compare these to truth/MC objects.

We'll build an analysis script/macro up bit-by-bit.  But if you're having
trouble placing the code snippets below, you can always refer to the
[full example scripts](../learners/full-example-script.md)
for guidance.

## Step 0: Skeleton

Overall, our strategy will proceed in two steps:

1. Match tracks to ECal clusters and identify electrons with an E/p cut; and then
2. Form pairs of electrons and identify J/psi with an invariant mass cut.

The code below will illustrate how to do this using (1) Python and ROOT, and (2)
C++ and ROOT (both cases using PODIO).  In the future, we'll expand this tutorial
with some extra documents illustrating how to do the same analysis using alternative
analysis techniques.

So to begin, let's 1st create a file.  We will suppose the file is called `FindJPsi.py`
 (for the Python version) or `FindJPsi.cxx` (for the C++ version), but you are ---
of course --- free to name the file what you want.

::::::::::::::::::::::::::::::::::::::::::::: challenge

## Exercise: Skeleton

Now, in that file, let's create a skeleton.  For Python:

```python
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


    # Event loop ==============================================================

    for frame in reader.get("events"):

        # Grab all the collections we'll need
        particles    = frame.get("MCParticles");
        tracks       = frame.get("CentralCKFTracks");
        clusters_neg = frame.get("EcalEndcapNClusters");
        clusters_cen = frame.get("EcalBarrelClusters");
        clusters_pos = frame.get("EcalEndcapPClusters");
        associations = frame.get("CentralCKFTrackAssociations");


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

```

And for C++:

```c++
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

  }  // end event loop

  // Calculate efficiencies and save output ===================================

  output->cd();
  output->Write();
  output->Close();
  return;

}
```

Now, just to make sure there are no typos, try running your script with either

```bash
python FindJPsi.py
```

for Python. Or:

```bash
root -b -q FindJPsi.cxx
```

for C++.


:::::::::::::::  solution

Both commands should finish after a few seconds without printing anything: the skeleton
just reads each event and grabs the collections.  The C++ version will also create an
(almost) empty `jpsi_search_cxx.analysis.root`.  If you see an error about a missing input
file, check that you run the script from the directory holding the file downloaded during
Setup (or point `--infile` at it).

:::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::

During the event loop, notice how we 1st grab a `frame` and then grab all of
the relevant collections we need from it.  Like we mentioned in the previous
episode, a PODIO `Frame` aggregates all of the data for all collections in
a given category.  In this context, we can think of a single frame as an "event"
(a simulated diffractive event).  In our actual data-taking operations, a frame
might be a "timeframe" of streamed data.

A couple notes:

- For thsoe following along in Python who haven't seen this before, the chunk at
  the bottom under `if __name__ == "__main__":` is there just make it easy to
  specify command line arguments.
- You might also see how the options in both cases correspond to our 2 steps up
  above.  We'll see them in action soon.

Next, let's fill in our histograms.  We'll define all the ones we need for this
tutorial.  However, we strongly encoourage you to play around and add additional
ones!

::::::::::::::::::::::::::::::::::::::::::::: challenge

## Exercise: Histograms

Add the following code snippets somewhere near the top of your script/macro.
For example right after you open the PODIO reader or right after you create
your output file.

```python
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
```

```c++
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
```

And for those using Python, add the following at the very end of your
`FindJPsi()` function:

```python

    with ROOT.TFile(out_file, "recreate") as ofile:
        ofile.WriteObject(h_track_momentum_all)
        ofile.WriteObject(h_track_momentum_match)
        ofile.WriteObject(h_track_eta_all)
        ofile.WriteObject(h_track_eta_match)
        ofile.WriteObject(h_track_cluster_eop)
        ofile.WriteObject(h_invariant_mass)
        ofile.WriteObject(h_daughter_momentum_all)
        ofile.WriteObject(h_daughter_momentum_reco)
        ofile.WriteObject(h_daughter_eta_all)
        ofile.WriteObject(h_daughter_eta_reco)
        ofile.WriteObject(h_jpsi_momentum_all)
        ofile.WriteObject(h_jpsi_momentum_reco)
        ofile.WriteObject(h_jpsi_eta_all)
        ofile.WriteObject(h_jpsi_eta_reco)
``` 

Then run your script/macro to make sure there are no typos.


:::::::::::::::  solution

The script should run exactly as before, and the output file now contains all 14
histograms — still empty, since we haven't filled them yet.

:::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::

## Step 1: Find Electrons

Now let's start filling in our skeleton.  We'll first need some
helper functions.

::::::::::::::::::::::::::::::::::::::::::::: challenge

## Exercise: Electron-Finding Helpers

Copy and paste the following snippets into your code just
after you get the necessary collections.

```python

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
```

```c++

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
```

And rerun your codes to make sure everything still works.


:::::::::::::::  solution

Nothing visible should change yet: the helpers are defined but not used until the next
exercise.  If Python complains about indentation or C++ about a stray semicolon, compare
against the [full example scripts](../learners/full-example-script.md).

:::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::

For those following along in C++ who haven't seen these before, these are examples
of _lambdas_, "anonymous functions."  These are functions we declare inline and
can use like we would other objects.  They're great for small functionality that
you only need locally, or for passing functions as arguments to other functions.

Also note that the ROOT `XYZVector` (and similar vectors below) are examples of
ROOT's successor to the older `TVector3` and related, the [`GenVector`](https://root.cern/doc/v640/group__GenVector.html).  There are still some quirks with their implementation,
which you'll see as we work through this.

### Track-Cluster Matching

Now, let's add our loop over tracks and start trying to match tracks and
clusters.  We'll start by just considering the EEEMCal.

::::::::::::::::::::::::::::::::::::::::::::: challenge

## Exercise: Track Loop

Copy and paste these snippets just after our helper functions above.

```python
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
```

```c++
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
        const float distance = get_distance(trk_mom, clust_mom);

        if ((distance < match_cut) && (distance < best_distance)) {
          best_distance = distance;
          best_match    = cluster;
          used_clusters.insert(cluster.getObjectID());
        }
      }
```

As always, run your code after the changes to make sure things work.


:::::::::::::::  solution

With the example file, `h_track_momentum_all` should now fill with about 3300 entries
(there are 3284 central tracks in the 1159 events).  The matching histograms stay empty
until the electron-identification step below fills them.

:::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::

You'll see how we're comparing the distance between the track and
cluster vectors in eta-phi space.  There are 3 points to emphasize
on the matching here:

1. Notice how we apply a cut on distance with `match_cut: we shouldn't
   consider clusters that are too far away from our track.
2. We also want to make sure we're taking the _best_ cluster (i.e. the one
   closest to the track in eta-phi space), not just the first one that
   falls in our match cut.  That's why we update the match iteratively.
3. We also need to avoid double-counting clusters: we use a `set`
   (both in Python and C++) to keep tabs of which clusters we've used since
   it doesn't allow duplicates.

After confirming the changes work, we can be lazy and just copy-and-paste
the EEEMCal loop two more times to get the BIC and FEMC (updateing the
collection names, of course):

```python
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
                    used_clusters.add(cluster)

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
```

```c++
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
```

There are a couple of caveats here:

1. We're just comparing the track/cluster eta, phi at _the vertex_ in the
   above snippets.  This isn't necessarily the best approach: it would be
   more prefereable to _project_ the tracks to the calorimeters and compare
   the eta, phi at the projections.
2. And we actually have an EICrecon algorithm which does just this!  The
   corresponding branches for the 3 calorimeters here are:
       - `EcalEndcapNTrackClusterMatches`
       - `EcalBarrelTrackClusterMatches`
       - `EcalEndcapPTrackClusterMatches`

A final note before moving on to the electron identification: if you're
following the C++ and haven't seen `std::optional` before, it's a way
to express data that may _or may not_ be present.  We express the same
idea in Python snippets using `None`.  You can also do the same thing
with a true/false flag.

### Electron Identification

And now, let's use our matches to identify electrons using an E/p
(cluster energy over track momentum) cut:

::::::::::::::::::::::::::::::::::::::::::::: challenge

## Exercise: Electron Identification

Copy and paste the following snippets in your code just after the
last track-cluster matching loop.

```python
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
```

```c++
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
```

And then confirm the changes work.


:::::::::::::::  solution

Draw `h_track_cluster_eop` after the run: you should see a clear peak just below 1
(around 0.95--0.98), which is exactly the electron $E/p$ signature discussed below.
With the example file about 2200 of the 3284 tracks end up matched to a cluster.

:::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::

This works because electrons (and positrons) on average deposit a characteristic
amount of their energy in a given ECal.  Ideally, this should be roughly 100%
of their energy, which would mean $E/p \sim 1$ .  You should be able to see this
peak very clearly in your `h_track_cluster_eop` histogram.

So as a rough way to identify which tracks are electrons, we can check which
tracks fall within some window around this average value.  Here, we assume
the average is 1.

## Step 2: Find J/Psi

And now that we have our electron candidates, we can find our $J/\psi$
candidates.

::::::::::::::::::::::::::::::::::::::::::::: challenge

## Exercise: J/psi-Finding Helpers

Like with step 1, we'll start with setting up some helper functions.  Paste
these snippets just after your track loop is done.

```python
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
```

```c++
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
```

And, as always, confirm that it works ;)


:::::::::::::::  solution

As with the earlier helpers, nothing visible changes yet — the lambda/function is
defined but only used in the next exercise.

:::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::

We'll need the 4-vectors here to calculate the invariant mass for
pairs of electrons/positrons.  If the pair are the $J/\psi$ decay $e^{\pm}$ ,
then this will be mass of the $J/\psi$ (modulo detector effects and
analysis biases, of course).  So we'll calculate the invariant mass
for all our electron candidates!

::::::::::::::::::::::::::::::::::::::::::::: challenge

## Exercise: J/Psi Identification

Copy these snippets just after your new helper functions.

```python
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
```

```c++
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
```

Then, after rerunning the code (and ironing out any bugs), check the
`h_invariant_mass` histogram.  Do you see a peak near the $J/\psi$ mass of
3.0969 GeV?


:::::::::::::::  solution

Yes!  With the example file, `h_invariant_mass` collects roughly 1400 pairs and shows a
prominent peak in the 3.0--3.1 GeV region, right at the $J/\psi$ mass.  The remaining
combinatorial background is broad and small in comparison.

:::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::

Just like with the electrons, we'll select our reconstructed $J/\psi$ by
picking out the pairs of electrons that have an invariant mass which
falls in some range including the actual mass.  All three cuts applied
here are parameters that you can tweak to improve things like the purity
of our $J/\psi$ candidates or our efficiency!

The next episode will fold in comparisons against the truth level, which
will give you some tools to do this.

::::::::::::::::::::::::::::::::::::::::::::: keypoints

- We can match tracks to clusters based on distances in eta-phi space
- Care is needed to make sure you're picking out the relevant cluster
- Electrons can be identified with an E/p cut
- J/psi can be reconstructed by considering pairs of reconstructed electrons

:::::::::::::::::::::::::::::::::::::::::::::
