---
title: "Parent-Daughter Matching"
teaching: 5
exercises: 15
---

::::::::::::::::::::::::::::::::::::::::::::: questions

- How do I use relations/associations?

:::::::::::::::::::::::::::::::::::::::::::::

::::::::::::::::::::::::::::::::::::::::::::: objectives

- Compare reconstructed J/psi to truth J/psi using relations/associations

:::::::::::::::::::::::::::::::::::::::::::::

## Step 3: Match Truth-Reco J/psi

Now, remember how we found our $J/\psi$ last time: by picking out pairs of $e^{\pm}$
which had the right invariant mass.  To emphasize: we never measure the $J/\psi$
directly.  We only measure their decay products.  Meaning that our $J/\psi$ are
_dileptons_, pairs of leptons.

This means that we have 3 levels of information to keep track of when we're
trying to compare our reconstructed $J/\psi$ against the truth ones:

1. Which $J/\psi$ decayed to $e^{\pm}$ ?
2. Which decay $e^{\pm}$ are related?
3. And which tracks are associated to these decay $e^{\pm}$ ?

### Helpers and Bookkeepers

So let's set up some helpers and bookkeepers to do do that.

::::::::::::::::::::::::::::::::::::::::::::: challenge

## Exercise: Truth Helpers

Copy the following snippets just after your loop over electron-pairs.

```python
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
```

```c++
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
```

And confirm that the changes run.


:::::::::::::::  solution

The code should run exactly as before: the maps are declared but not filled until the
next exercise.

:::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::

Notice the dictionaries (Python) and maps (C++): these are what we'll use to keep
track of the 3 levels of info listed above.  There are many ways you can do this,
and this way is somewhat clunky.  But it is transparent and conveys how to use
these structures for these types of problems.

### Find Decay Products

Now, let's fill these structures by finding the daughters (decay $e^{\pm}$ ) and their
corresponding tracks.

::::::::::::::::::::::::::::::::::::::::::::: challenge

## Exercise: Find Decay Products

Copy the following snippets just after your new helpers/bookkeepers.

```python
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
```

```c++
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
```

And confirm that the code still runs!


:::::::::::::::  solution

After this step `h_daughter_momentum_all` should hold two entries per event (both decay
leptons; 2318 in the example file), and `h_daughter_momentum_reco` about 92% of that —
most decay electrons are successfully reconstructed as tracks.

:::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::

In particular, notice how easily we were able to (1) find the relevant daughters of the
$J/\psi$ and (2) how easily we handled the mc-track associations.  This is one of the
biggest advantages using the PODIO interface brings over analysis methods.

### Compare Truth vs. Reco

We're now in the home stretch!  All that's left to do is compare truth vs. reco using
the structures we've already filled.  For now, all we're going to do is calculate our
$J/\psi$ reconstruction efficiency.  This folds in 3 sources of efficiency:

1. Were our decay $e^{\pm}$ reconstructed as tracks?
2. Were these tracks successfully matched to clusters?
3. And how much did our analysis cuts eat into our signal?

::::::::::::::::::::::::::::::::::::::::::::: challenge

## Exercise: Truth-Reco Comparison

Copy and paste these snippets in your code.

```python
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
```

```c++
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
```

And check that the code runs.


:::::::::::::::  solution

With the example file, `h_jpsi_momentum_reco` ends up with about 190 entries out of the
1159 generated $J/\psi$, i.e. a reconstruction efficiency of roughly 17%.  The next
section computes these efficiencies properly, and the following section discusses why
this number is so rough.

:::::::::::::::

:::::::::::::::::::::::::::::::::::::::::::::

Finally, let's calculate the efficiencies.  You can do so by copying the following
snippets into your code just after the event loop.

```python
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
```

```c++
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
```

Don't forget to save your new histograms in the Python case! You now have a complete analysis
script!

## Next steps

If you look at our $J/\psi$ reconstruction efficiency, it's going to be pretty rough.  I'd
encourage you to play around with the code and see if you can improve things!

Hopefully, all of this gives you an idea of how the PODIO interface works and has shown
how it help unlock relatively complicated analyses!

::::::::::::::::::::::::::::::::::::::::::::: keypoints

- J/psi are measured as dileptons, pairs of decay leptons
- Need to track which J/psi decayed to which electrons and their associated tracks

:::::::::::::::::::::::::::::::::::::::::::::
