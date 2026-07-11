---
title: 'Instructor Notes'
---

# Outline

Key points:
  - Illustrate basics of PODIO usage in
    C++, Python
  - Illustrate navigating links, associations,
    and relations

Skeleton:

1. Introduction
    - Goal of tutorial
      - PODIO basics and using it in an analysis
      - Via track-cluster matching and parent-
      daughter matching
    - Track-cluster Matching: illustrate PODIO
      basics + links
      - Paradigmatic of a lot of analysis operations
      - First step towards particle/jet/event
        reconstruction
      - e.g. reconstructing a J/psi, you need
        to ID electrons
    - Parent-Daughter Matching: illustrate
      navigating relations
      - Truth-level parallel of track-cluster
        matching
      - e.g. need to validate our reconstructed
        J/psi

2. PODIO
    - Framework for generating, managing EDMs
    - Defined in a .yml file (include link)
    - Navigating the model:
      - Example: Cluster
        - Explanation of fields, "guessing" the
          setter/getters
        - Note: doxygen and /opt/include
      - Example: Track
      - Example: MCParticle 
    - Relations: "adjacent" connections between
      objects
      - Encode "causal" connections
      - Or "combinatorial" connections
    - Links/Associations: "orthogonal" connections
      - Encodes connections which may (or may not)
        be present

3. Track-cluster matching
    - (A) Set up basic script
      - Show track-cluster matching efficiency 
    - (B) Select e+- based on E/p
    - (C) Plot invariant mass
    - Note: have algorithm to do this in EICrecon,
      can make use of premade branches

4. Parent-daughter matching
    - (A) Set up script/macro
    - (B) Identify FS e+-
    - (C) Navigate decay chain to find parent
      J/psi
    - (D) Plot some info of J/psi vs. decay
      (and maybe truth vs. candidate reco
      J/psi?)

5. Extras: how do same analysis with other
   methods
    - RDataFrame (with PODIO)
    - RDataFrame (pure ROOT, Python)
    - TTreeReader
    - Uproot + Awkward (STRETCH GOAL)
