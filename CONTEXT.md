# CONTEXT.md — orientation and history

Background an agent or a new contributor needs that does not belong in
`AGENTS.md`: domain notes, current state, glossary, open questions. Read the
section a task needs, not the whole file.

## Current state

Set up to the bar of swarm-routing/ARA and swarm-routing/HOPNET (#36, part of
swarm-routing/.github#881): agent files, CITATION.cff, the release ladder
(`docs/roadmap.md`), the release workflow, `docs/references.bib` and a first
`docs/fidelity.md`. Neither paper has a known open copy (OpenAlex: closed,
2026-11-24); the journal DOI resolves to dblp's index because the publisher's
site is gone. By the maintainer's decision of 2026-11-24 (#24), the sheet is
v0 from secondary sources: the first author's open master's thesis (Concordia
University, 2007, ch. 5; a scan without a text layer, read from the page
images) and an open review. `core/` (epic #37, first slice #38) is built from
it after the sheet's review PR.

## Glossary

- **Position-based routing**: routing that uses the geographic positions of
  nodes (e.g. from GPS) to choose next hops.
- **ACO**: ant colony optimization; ants sample paths and leave pheromone that
  biases later choices.
- **Zones**: a node's neighbours split by the angle θ between the vectors to
  the neighbour and to the destination: zone 1 θ ≤ π/4, zone 2 between,
  zone 3 θ ≥ 3π/4 (thesis §5.1). Pheromone starts higher in zone 1.
- **BR table**: back routing; where a forward ant came from, so the backward
  ant retraces the path.

## Open questions

- Which version is primary: the 2008 journal paper unless it turns out to
  specify less than the 2007 conference paper (swarm-routing/.github#672,
  decision 18; #24).
- Whether the papers' simulations match the thesis's (AntNet, AntHocNet and
  GPSR on static graphs with irregular ranges), and so what the anchor of
  org golden rule 6 can be.
