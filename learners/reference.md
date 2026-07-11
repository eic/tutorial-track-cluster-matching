---
title: 'Reference'
---

## Glossary

PODIO
:   Plain-Old-Data Input/Output — a toolkit for generating and managing event data models
    like EDM4eic.  It reads a YAML description and generates the C++ and Python code used
    to read, write, and interface with the data.

EDM4eic
:   The Event Data Model for the EIC — the standardized set of data structures (tracks,
    clusters, associations, ...) used throughout the ePIC software stack.  Defined in a
    single YAML file, [edm4eic.yaml](https://github.com/eic/EDM4eic/blob/main/edm4eic.yaml).

EDM4hep
:   The Key4hep project's event data model, also used in ePIC software (e.g. for
    `MCParticle`s and common components like `Vector3f`).

Class
:   A data-model structure with members accessed via `get`/`set`-prefixed accessors
    (e.g. `track.getMomentum()`).

Component
:   A simple `struct` used inside classes (e.g. `edm4hep::Vector3f`); its members are
    accessed directly, without `get`/`set` prefixes (e.g. `momentum.x`).

Collection
:   A read-only container of data-model objects that can be written to and read from file
    (e.g. `edm4eic::TrackCollection`).

Frame
:   The PODIO container that holds and organizes all collections for one event (or, in
    streaming contexts, one timeframe).

Relation
:   A direct reference from one data-model object to one (`OneToOneRelations`) or many
    (`OneToManyRelations`) other objects, e.g. from a cluster to its calorimeter hits.

Association
:   A standalone object expressing an indirect, possibly absent connection between two
    objects, e.g. `edm4eic::MCRecoParticleAssociation` between a Monte-Carlo particle and
    its reconstructed counterpart.

Link
:   The successor of associations: a lightweight directional connection with fixed
    `from`/`to`/`weight` fields.  Associations are being deprecated in favor of links.

E/p
:   The ratio of a matched cluster's energy to the track's momentum.  Electrons and
    positrons deposit essentially all their energy in the electromagnetic calorimeter,
    so their tracks peak at $E/p \approx 1$.
