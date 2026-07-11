---
title: Setup
---

If you have not done so already, please follow the
[environment setup tutorial](https://eic.github.io/tutorial-setting-up-environment/)
well before the start of the tutorial to ensure your system is ready.  You will need a working
eic-shell environment running.

This tutorial will cover the basics of [PODIO](https://github.com/AIDASoft/podio) and illustrate how to use it in an analysis setting
through the lens of a critical operation in our reconstruction, matching tracks to clusters.  We will also touch on other examples of how
to use PODIO in your analysis such as identifying the decay electrons of a $J/\psi$.  Examples of how to do the same analyses in other
formats --- such as with `RDataFrame` or with `TTreeReader` --- are included as extras at the end of the tutorial (**TODO**).

We will need a file to process for the example code, so if you havent done so yet, please follow the
[tutorial on accessing EIC/ePIC simulation data](https://eic.github.io/tutorial-file-access/) with Rucio.
We suggest using a file with an enhanced rate of $J/\psi$.  For example, you can run the following command in the eic-shell:

<style>
pre.bash code { white-space: pre-wrap; word-break: break-all; }
</style>

```bash
xrdcp root://epicxrd1.sdcc.bnl.gov:1095//eic/EPIC//RECO/26.04.1/epic_craterlake/EXCLUSIVE/DIFFRACTIVE_JPSI_ABCONV/lAger3.6.1-1.0/10x130/hiAcc/lAger3.6.1-1.0_jpsi_10x130_hiAcc_run1.0009.eicrecon.edm4eic.root ./
```

Lastly, you may want to work through some sections of the
[introductory analysis tutorial](https://eic.github.io/tutorial-analysis/).
In particular, the episode on
[the reconstruction output tree](https://eic.github.io/tutorial-analysis/02-reconstruction-output.html)
touches on the structure of the ROOT trees we'll be analyzing.
