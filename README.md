# EIC Tutorial: Track-Cluster Matching and PODIO

[![The Carpentries Workbench](https://img.shields.io/badge/Built%20with-The%20Carpentries%20Workbench-071159.svg)](https://carpentries.github.io/sandpaper-docs/)

An ePIC tutorial that introduces the PODIO interface to the EDM4eic data model and shows how to
use it in an analysis setting: matching tracks to calorimeter clusters, identifying electrons
with an E/p cut, reconstructing J/psi from electron pairs, and validating the result against
Monte-Carlo truth via relations and associations.

This lesson is built with [The Carpentries Workbench](https://carpentries.github.io/sandpaper-docs/).

## Building the lesson locally

The lesson is rendered with the Workbench Docker image (no local R installation needed). A
`Makefile` wraps the commands:

```bash
make preview   # build the site into site/docs/
make serve     # serve site/docs/ at http://localhost:4321
make clean     # drop the build cache
```

## Contributing

We welcome all contributions to improve the lesson! Please familiarize yourself with our
[Contribution Guide](CONTRIBUTING.md). For the Workbench Markdown syntax (callouts, challenges,
etc.) see the [sandpaper documentation](https://carpentries.github.io/sandpaper-docs/).

## Maintainer(s)

Current maintainers of this lesson are

- Derek Anderson
