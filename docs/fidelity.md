# POSANT: fidelity sheet

What the papers specify, what this implementation is to do, and where they
differ. Every row cites its source (`[KEY] §x`, `Fig. N`, `eq. N`, `p. N`),
or says `repo choice: <reason>`, or `unknown`. Checked by
`swarm-routing/.github` `tools/fidelity/fidelity.py check`.

**State: started, not reviewed.** Neither paper has a known open copy
(OpenAlex lists both as closed on 2026-11-24; the journal DOI resolves to
dblp's index because the publisher's site is gone), and this sheet was
written without either text. What the two **abstracts** state (public
metadata, read from OpenAlex) is noted in the rows' status, but an abstract
is no locator, so those rows stay `unknown` as well. Every row is `unknown`
or a repo choice until someone with the texts reads them (#24, a maintainer
step). No protocol code is written before the maintainer reviews
this sheet (AGENTS.md rule 11; #37, #38).

## Sources

<!-- fidelity:sources:begin -->
| Key | Citation | Link |
|---|---|---|
| KO07 | Kamali & Opatrny, *POSANT: A Position Based Ant Colony Routing Algorithm for Mobile Ad-hoc Networks*, 2007 Third International Conference on Wireless and Mobile Communications (ICWMC'07), 2007, pp. 21–21 | <https://doi.org/10.1109/icwmc.2007.68> |
| KO08 | Kamali & Opatrny, *A Position Based Ant Colony Routing Algorithm for Mobile Ad-hoc Networks*, Journal of Networks 3(4), 2008 | <https://doi.org/10.4304/jnw.3.4.31-41> |
<!-- fidelity:sources:end -->

The entries live in [`references.bib`](references.bib); the table is
generated from it (`fidelity.py bib sources`).

## Which version of POSANT

Two texts describe POSANT:

- **The journal paper**, [KO08] (Journal of Networks 3(4), 2008). The
  primary source: the published, longer version, which is expected to specify
  more of the state and the defaults (swarm-routing/.github#672, decision 18).
  Not read.
- **The conference paper**, [KO07] (ICWMC 2007). Kept for the deviations:
  any place where it disagrees with [KO08] gets a row and a stated choice of
  version. Not read.

If [KO08] turns out to specify less than [KO07], the primary is revisited on
#24 before any row is filled. There is no AntHocNet arm to compare with:
POSANT was never implemented in AntHocNet, so there is no *Differences from*
section.

## Mechanisms

| Mechanism | Source | Implementation | Status |
|---|---|---|---|
| A reactive routing algorithm: routes are established on demand, from a source to a destination | unknown | Route discovery started by a data packet with no route, as the paper specifies | unknown: both abstracts state it; no section read, so the trigger and its buffering are open |
| Combines ant colony optimization with the positions of nodes | unknown | Positions from swarm-core's `ILocationProvider` (`core/` never reads a simulator, AGENTS.md rule 10) | unknown: both abstracts state it; how the positions enter the ants' choice is not in them |
| How a node learns its neighbours' and the destination's positions (beacons, a location service, carried in the ants) | unknown | — | unknown: in neither abstract |
| The division of a node's neighbours into zones by direction relative to the destination ("directional zoning", swarm-routing/.github#672) | unknown | — | unknown: the plan's description, not yet read in either text; the number of zones and their boundaries are `unknown` |
| Forward ants: when they are sent, how many, to which neighbours | unknown | — | unknown |
| Backward ants and the pheromone update on the way back | unknown | — | unknown |
| The next-hop choice of ants and of data from pheromone (and position) | unknown | swarm-core's `PheromoneTable` and roulette selection if they fit, through an injected RNG (org rule 5) | unknown |
| Pheromone evaporation | unknown | — | unknown |
| Ant and data headers: their fields | unknown | One POD header per type, little-endian, leading version byte (org rule 3); the field widths are ours | unknown (layout: repo choice under org rule 3) |
| Duplicate ants: how they are detected and dropped | unknown | Bounded (source, seq) history (org rule 7) | unknown |
| Route maintenance and link failure | unknown | — | unknown |
| Behaviour where no neighbour lies toward the destination (a void), or with irregular transmission ranges, where the abstract says other position-based algorithms fail | unknown | — | unknown: the abstracts state the problem as motivation, not POSANT's mechanism for it |
| 3D: the zoning and the choice in three dimensions | repo choice: UAV swarms and FANETs move in 3D (README, #3); the papers are taken to be 2D until read | Generalised from the 2D rows once they are reviewed; its own rows, default-off against the 2D reading | repo choice (pending: no row to generalise yet) |

## Parameters

| Parameter | Value | Source | Notes |
|---|---|---|---|
| Number of directional zones | — | unknown | |
| Pheromone initial value, increment and evaporation rate | — | unknown | |
| Forward-ant count or rate per route discovery | — | unknown | |
| Ant time-to-live or hop limit | — | unknown | |
| Selection exponents or weights (pheromone, position) | — | unknown | |
| Route discovery timeout and retries | — | unknown | |
| Duplicate-history size | a configured cap | repo choice: org rule 7 keeps bounded structures bounded | the number is set with #38 |

## Anchor candidate

| Trend | Scenario | Metric | Comparators | Source |
|---|---|---|---|---|
| POSANT establishes routes faster than other ant colony routing algorithms | unknown | route establishment time | "other ant colony routing algorithms" (which ones: unknown) | unknown |
| POSANT sends fewer control messages than other ant colony routing algorithms | unknown | control messages | as above | unknown |

The anchor (org golden rule 6) needs the papers' scenario, the comparators
they ran and the figures' trends; all three are `unknown` until the texts are
read. If a comparator is AntHocNet or ARA, the org's implementations are the
natural arm (`swarm-routing/ns3-tools`).

## Open items for the review

- Read [KO08] and [KO07] against every `unknown` row above (#24); a row
  either gets a locator, or a `repo choice: <reason>` accepted on an issue
  first, or stays `unknown` with the reason.
- Confirm the primary version (decision 18), and list where [KO07] differs.
- Fill the anchor candidate from the papers' evaluation section.
