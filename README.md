# POSANT

<!-- org-sync:begin badges -->
| Group | Badges |
| --- | --- |
| Build | [![Built on: ns3-dev](https://img.shields.io/badge/built%20on-ns3--dev%20image-blue?logo=docker)](https://github.com/orgs/swarm-routing/packages/container/package/ns3-dev) |
| Quality | [![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=swarm-routing_POSANT&metric=alert_status)](https://sonarcloud.io/summary/new_code?id=swarm-routing_POSANT) [![Bugs](https://sonarcloud.io/api/project_badges/measure?project=swarm-routing_POSANT&metric=bugs)](https://sonarcloud.io/summary/new_code?id=swarm-routing_POSANT) [![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=swarm-routing_POSANT&metric=security_rating)](https://sonarcloud.io/summary/new_code?id=swarm-routing_POSANT) [![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=swarm-routing_POSANT&metric=sqale_rating)](https://sonarcloud.io/summary/new_code?id=swarm-routing_POSANT) [![CodeRabbit Pull Request Reviews](https://img.shields.io/coderabbit/prs/github/swarm-routing/POSANT?utm_source=oss&utm_medium=github&utm_campaign=swarm-routing%2FPOSANT&labelColor=171717&color=FF570A&link=https%3A%2F%2Fcoderabbit.ai&label=CodeRabbit+Reviews)](https://coderabbit.ai) |
| Release | [![Release](https://img.shields.io/github/v/release/swarm-routing/POSANT?sort=semver&cacheSeconds=1800)](https://github.com/swarm-routing/POSANT/releases) [![Cite](https://img.shields.io/badge/cite-CITATION.cff-blue)](CITATION.cff) [![Licence: GPL-2.0-only, core/ MIT OR GPL-2.0-only](https://img.shields.io/badge/licence-GPL--2.0--only%2C%20core%2F%20MIT%20OR%20GPL--2.0--only-blue)](LICENSING.md) |
| Trust | [![OpenSSF Scorecard](https://api.securityscorecards.dev/projects/github.com/swarm-routing/POSANT/badge)](https://scorecard.dev/viewer/?uri=github.com/swarm-routing/POSANT) [![OpenSSF Best Practices](https://www.bestpractices.dev/projects/17067/badge)](https://www.bestpractices.dev/projects/17067) |
| Stack | [![C++23](https://img.shields.io/badge/C%2B%2B-23-blue?logo=cplusplus)](AGENTS.md) [![Runs on ns-3](https://img.shields.io/badge/runs%20on-ns--3-informational)](https://www.nsnam.org) |
| Community & docs | [![Colony Arena: POSANT](https://img.shields.io/badge/Colony%20Arena-project%20page-orange)](https://swarm-routing.github.io/projects/POSANT/) [![Open RCAPA](https://img.shields.io/github/issues/swarm-routing/POSANT/rcapa?label=open%20RCAPA&color=blue)](https://github.com/swarm-routing/POSANT/blob/main/docs/rcapa/README.md) [![Conventional Commits](https://img.shields.io/badge/Conventional%20Commits-1.0.0-yellow)](https://www.conventionalcommits.org) |
<!-- org-sync:end badges -->

POSANT (position-based ant colony routing) as a standalone swarm routing protocol for geographic and 3D networks: UAV swarms, FANETs and GPS-equipped outdoor meshes. It will use swarm-core's `ILocationProvider` port for positions and directional zoning in 2D and 3D.

POSANT is the algorithm of Kamali & Opatrny ([ICWMC 2007](https://doi.org/10.1109/icwmc.2007.68); [Journal of Networks, 2008](https://doi.org/10.4304/jnw.3.4.31-41)): a reactive ant colony routing algorithm that combines ant colony optimization with the positions of nodes.

**Status: set up, no protocol code yet.** The fidelity sheet ([docs/fidelity.md](docs/fidelity.md)) comes first and is reviewed before any of `core/` is written; the release ladder is [docs/roadmap.md](docs/roadmap.md). The plan is #3 and swarm-routing/.github#672.

<!-- org-sync:begin develop -->
## Develop

New here? [docs/development.md](docs/development.md) is the developer guide:
the tools to install, how to get the code and its dependencies, how to
build, test, run and lint it, the Docker images, what CI checks and how to
read a red run. How changes are proposed, reviewed and released is the org's
[CONTRIBUTING.md](https://github.com/swarm-routing/.github/blob/main/CONTRIBUTING.md).
Whether a regression here gets an RCAPA (root cause analysis with corrective
and preventive actions), and how: [docs/rcapa/README.md](docs/rcapa/README.md).
<!-- org-sync:end develop -->

<!-- org-sync:begin citing -->
## Releases & citing

Versions follow [SemVer](https://semver.org). Each release is listed under [Releases](https://github.com/swarm-routing/POSANT/releases), built by the org's release workflow and archived on [Zenodo](https://zenodo.org) with its own DOI.

To cite this repository, use the “Cite this repository” button (from [CITATION.cff](CITATION.cff)). It has no DOI yet: the concept DOI appears here and in the badge above once Zenodo archives its first release.

Please also cite the work it builds on:

- S. Kamali, J. Opatrny, *A Position Based Ant Colony Routing Algorithm for Mobile Ad-hoc Networks*, Journal of Networks 3(4), 2008, [doi:10.4304/jnw.3.4.31-41](https://doi.org/10.4304/jnw.3.4.31-41). The journal version of POSANT, the primary source this repository implements (swarm-routing/.github#672, decision 18).

What this implementation reproduces from its sources, and every deliberate deviation, is in [docs/fidelity.md](docs/fidelity.md).

Using it in your work? Contributions are welcome, and citing it helps just as much.
<!-- org-sync:end citing -->

## Epics

<!-- epics:begin -->
<!-- Generated by swarm-routing/.github tools/epics/epics.py from the issues labelled `epic`; edit the issues, then re-run it. -->

What is planned and in progress here: the issues labelled [`epic`](https://github.com/swarm-routing/POSANT/issues?q=label%3Aepic). An epic's open sub-issues carry a `priority:*` and a `model:*` label, so a contributor or an AI agent can pick one up ([how issues work](https://github.com/swarm-routing/.github/blob/main/CONTRIBUTING.md#issues)). The whole org's plan: the [roadmap](https://github.com/swarm-routing/.github/blob/main/ROADMAP.md#epics).

| Epic | Status | Sub-issues closed |
|------|--------|-------------------|
| [#3](https://github.com/swarm-routing/POSANT/issues/3) Bootstrap POSANT protocol on swarm-core (DirectionalZoning 2D/3D, fidelity, 3D UAV benchmarks) | Planned | 0 / 1 |

<!-- epics:end -->

## Licence

GPL-2.0-only, with `core/` MIT OR GPL-2.0-only: see [LICENSING.md](LICENSING.md).
