# CONTEXT.md — orientation and history

Background an agent or a new contributor needs that does not belong in
`AGENTS.md`: domain notes, current state, glossary, open questions. Read the
section a task needs, not the whole file.

## Current state

Set up to the bar of swarm-routing/ARA and swarm-routing/HOPNET (#36, part of
swarm-routing/.github#881): agent files, CITATION.cff, the release ladder
(`docs/roadmap.md`), the release workflow, `docs/references.bib` and a first
`docs/fidelity.md`. No protocol code: `core/` (epic #37, first slice #38)
waits for the fidelity sheet's review, which waits for the papers' texts
(#24). Neither paper has a known open copy (OpenAlex: closed, 2026-11-24);
the journal DOI resolves to dblp's index because the publisher's site is gone.

## Glossary

- **Position-based routing**: routing that uses the geographic positions of
  nodes (e.g. from GPS) to choose next hops.
- **ACO**: ant colony optimization; ants sample paths and leave pheromone that
  biases later choices.
- **Directional zoning**: the division of a node's neighbours into zones by
  their direction relative to the destination, as this repository's plan
  describes POSANT (swarm-routing/.github#672); its exact form in the papers
  is `unknown` in `docs/fidelity.md` until the texts are read.

## Open questions

- Which version is primary: the 2008 journal paper unless it turns out to
  specify less than the 2007 conference paper (swarm-routing/.github#672,
  decision 18; #24).
- What the papers' simulations compared against, and so what the anchor of
  org golden rule 6 can be.
