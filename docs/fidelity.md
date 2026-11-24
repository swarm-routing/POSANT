# POSANT: fidelity sheet

What the sources specify, what this implementation does, and where they
differ. Every row cites its source (`[KEY] §x`, `Fig. N`, `eq. N`, `p. N`),
or says `repo choice: <reason>`, or `unknown`. Checked by
`swarm-routing/.github` `tools/fidelity/fidelity.py check`.

> **v0 from secondary sources.** This sheet, and the `core/` built from it,
> follow the maintainer's decision of 2026-11-24 on
> [#24](https://github.com/swarm-routing/POSANT/issues/24#issuecomment-7147106974)
> (recorded org-wide on swarm-routing/.github#881): a protocol whose papers are
> not openly available is built first from public sources. Neither POSANT paper
> has been read ([KO08] and [KO07]: OpenAlex lists both as closed). The sheet is
> written from the first author's open master's thesis [K07t], whose ch. 5
> specifies POSANT and cites the conference paper as its own [19], cross-checked
> against the open review [CIS19] §2.3.1. **No row claims fidelity to the
> papers.** The anchor gate (org golden rule 6) stays open, and nothing is
> released on this basis.

Every row's **Status** names its basis: `paper` (read in [KO08] or [KO07]:
none yet), `secondary: [KEY]` (read in an open source), or `repo choice`.
Locators for [K07t] are its printed page numbers (the PDF page is the
printed page + 12) and its section, equation, figure and algorithm numbers.
The thesis PDF is a scan with no text layer; every value below was read from
the page image.

## Sources

<!-- fidelity:sources:begin -->
| Key | Citation | Link |
|---|---|---|
| CIS19 | Habboush, *Ant Colony Optimization (ACO) Based MANET Routing Protocols: A Comprehensive Review*, Computer and Information Science 12(1), 2019, p. 82 | <https://doi.org/10.5539/cis.v12n1p82> |
| K07t | Kamali, *Position based ant colony optimization routing in mobile ad hoc networks*, Master's thesis, Concordia University, 2007 | <https://spectrum.library.concordia.ca/id/eprint/975539/> |
| KO07 | Kamali & Opatrny, *POSANT: A Position Based Ant Colony Routing Algorithm for Mobile Ad-hoc Networks*, 2007 Third International Conference on Wireless and Mobile Communications (ICWMC'07), 2007, pp. 21–21 | <https://doi.org/10.1109/icwmc.2007.68> |
| KO08 | Kamali & Opatrny, *A Position Based Ant Colony Routing Algorithm for Mobile Ad-hoc Networks*, Journal of Networks 3(4), 2008 | <https://doi.org/10.4304/jnw.3.4.31-41> |
<!-- fidelity:sources:end -->

The entries live in [`references.bib`](references.bib); the table is
generated from it (`fidelity.py bib sources`).

## Which version of POSANT

- **The thesis**, [K07t] ch. 5 (pp. 31–52): the algorithm, its parameters
  (Table 1) and its evaluation against AntNet, AntHocNet and GPSR. This sheet
  is written from it.
- **The conference paper**, [KO07], and **the journal paper**, [KO08]: the
  published versions. [KO08] stays the intended primary (swarm-routing/.github#672,
  decision 18). Neither is read; every row below waits on them for the
  `paper` basis (#24), and any place where they differ from [K07t] gets a
  deviation and a stated choice of version.
- **The review**, [CIS19] §2.3.1: an independent summary. It agrees with
  [K07t] on the three zones, the forward and backward ants (FANT/BANT), the
  pheromone and back-routing (BR) tables, the stochastic next hop and the
  regular and broken modes; it adds nothing [K07t] does not state.

## Mechanisms

| Mechanism | Source | Implementation | Status |
|---|---|---|---|
| Each node knows its own position, its neighbours' positions and the destination's position | [K07t] p. 9; [CIS19] §2.3.1 | The adapter hands the core its own position, each neighbour's and the destination's; `core/` never reads them from a simulator (AGENTS.md rule 10) | as written; secondary: [K07t] |
| Reactive: a route is searched only when data are to be sent from S to D; data are sent only after a route is established, and before that only forward and backward ants are exchanged | [K07t] p. 31, p. 10 | Route establishment starts on the first data packet for a destination with no pheromone | as written; secondary: [K07t] |
| Zones: for node S and destination D, neighbour H is in zone 1 if θ_H ≤ π/4, zone 2 if π/4 < θ_H < 3π/4, zone 3 if 3π/4 ≤ θ_H ≤ π, θ_H the angle between SH and SD | [K07t] §5.1, p. 32, Fig. 11 | `zoneOf()` from the angle between the two vectors (dot product, `acos`) | as written; secondary: [K07t] |
| The angle in 3D | repo choice: [K07t] p. 8 projects a 3D network on the ground when heights are similar; UAV swarms are not (#3) | The same angle between SH and SD as 3D vectors: the zones become cones about SD; with all z equal it is exactly the 2D rule | repo choice |
| A node with H at its own position, or D at its position (zero-length vector) | repo choice: the angle is undefined there | Such a neighbour is in zone 1 | repo choice |
| Pheromone table: a pheromone value per outgoing link and destination | [K07t] §5.2, p. 33 | Table keyed (destination, neighbour) | as written; secondary: [K07t] |
| Pheromone initialisation: when a node gets a packet for D and has no trail for D, every outgoing link AB gets ν_i, B in zone i, ν1 ≥ ν2 ≥ ν3 | [K07t] §5.2, p. 33 | `initialise()` on first use for D, from the node's current neighbours | as written; secondary: [K07t] |
| Trails for D (and their BR entries) are deleted when no packet for D arrives for a time "in the order of seconds" | [K07t] §5.2, p. 33 | Idle timeout per destination, `Config::trailTimeout` | as written (value: repo choice, below) |
| A neighbour that appears after a destination's trails exist | unknown | It gets ν of its zone when first seen for that destination | unknown: [K07t] ch. 5 does not say (HYBNET's §6.5 "Handling new links" is a different algorithm) |
| Next hop of a forward ant or data packet: neighbour H_i with probability p_i = φ_i / Σ_{j=1..k} φ_j over the k neighbours | [K07t] §5.3, eq. 2, p. 35, Alg. 2, p. 35 | Roulette over the trails of the current neighbours through an injected RNG (org rule 5) | as written; secondary: [K07t] |
| The previous hop is a candidate too | [K07t] eq. 2, p. 35 | Not excluded (eq. 2 sums over all k neighbours); loops end at the duplicate rule | as written; secondary: [K07t] |
| The source launches n forward ants with unique sequence numbers from each of its three zones at regular intervals (3n per interval), n = 1 | [K07t] §5.3, p. 35, Alg. 1, p. 34 | Per tick, per zone with neighbours and not stopped: n forward ants whose first hop is drawn by eq. 2 among that zone's neighbours | as written; secondary: [K07t]. "From each zone" read as the first hop in that zone |
| Back Routing (BR) table: a forward ant entering a node stores (previous hop, sequence number, destination) | [K07t] §5.3, p. 36; [CIS19] §2.3.1 | BR table keyed (source, sequence number), bounded (org rule 7) | as written; secondary: [K07t]. The source in the key is a repo choice: sequence numbers are per source |
| Repeated forward ants are destroyed | [K07t] §5.3, p. 36 | A forward ant whose (source, seq) is already in the BR table is dropped | as written; secondary: [K07t] |
| A forward ant reaching D is destroyed and a backward ant with the same sequence number goes back on the same path, using the BR tables | [K07t] §5.3, p. 36, Alg. 1, p. 34 | The destination answers each forward ant (duplicates excepted) | as written; secondary: [K07t] |
| Backward ant update moving from B to A: φ_AB = φ_AB + ω(AB) · g(d), d the length of the path the backward ant travelled from D to B, g decreasing | [K07t] §5.3, eq. 3, p. 36 | Applied at A when the backward ant arrives from B | as written; secondary: [K07t] |
| d at B = D (the first hop back) is 0, where g(d) = 1000/d² is undefined | repo choice: eq. 3 and Table 1 leave d = 0 undefined | d counts hops from D to B, and g uses max(d, 1) | repo choice |
| ω(AB): w1 ≥ 1 if B is in zone 1 of A, w2 = 1 in zone 2, w3 ≤ 1 in zone 3 | [K07t] §5.3, eq. 4, p. 36 | `Config::weight` per zone | as written (values: unknown, below) |
| Evaporation: every trail is multiplied by μ < 1 at regular intervals | [K07t] §5.3, eq. 5, p. 37; [K07t] Alg. 1, p. 34 | Once per tick, every trail of every destination | as written; secondary: [K07t] |
| Each backward ant carries the length of the path from D to its current hop; at the source it updates that zone's average α_i and standard deviation σ_i of delays over a window of the latest backward ants, then is dropped | [K07t] §5.4, p. 38, Alg. 1, p. 34 | A fixed-size window per (destination, zone) at the source | as written; secondary: [K07t] |
| The delay of a packet is its hop count | [K07t] p. 9, §5.4, p. 38 | Hop counts throughout | as written; secondary: [K07t] |
| If σ_i < t, stop forward ants to zone i and send data packets instead | [K07t] §5.4, pp. 38–39 | Per zone: ants until converged, then data | as written; secondary: [K07t] |
| If σ_i, σ_j < t and α_j > α_i + c, stop ants and data to zone j and remove the source's trails for D in zone j | [K07t] §5.4, p. 39 | Zone j closed at the source until the route is re-established | as written (c: unknown, below) |
| An empty window (no backward ant yet) | repo choice: σ is undefined | The zone is not converged | repo choice |
| Data packets follow the same process as forward ants; the source sends one data packet from each converged zone per tick | [K07t] §5.4, p. 39, Alg. 1, p. 34 | Data next hops by eq. 2; each tick the source sends data from each converged, open zone | as written; secondary: [K07t] |
| Data buffered at the source before a route converges | unknown | Held in a bounded queue (org rule 7), oldest dropped when full | unknown: [K07t] does not say what happens to data before convergence |
| Modes: every node is in regular mode; a node whose link to a neighbour breaks enters broken mode | [K07t] §5.5, p. 40; [CIS19] §2.3.1 | `Mode` per node | as written; secondary: [K07t] |
| On a broken link AB with trails for D on other links: routing continues on them | [K07t] §5.5, p. 40 | The broken link's trails are removed for every destination | as written; secondary: [K07t]. Removing the broken link's trails is a repo choice the text implies |
| With no trail for D left: A tells its neighbours there is no route to D from A; they act as if their link to A were broken | [K07t] §5.5, p. 40 | A no-route message per destination; the receiver removes its trail to A for D and repeats the rule | as written; secondary: [K07t] |
| Loop guard: a node in broken mode that receives a packet for the second time sends the no-route message | [K07t] §5.5, p. 40 | A packet seen again (same source and sequence number) in broken mode triggers it | as written; secondary: [K07t] |
| The source losing its last trail for D (a break, or a no-route message) restarts route establishment and suspends data until a new route is found | [K07t] §5.5, p. 40 | The source's per-destination state is reset to establishing | as written; secondary: [K07t] |
| Broken mode returns to regular a time "in the order of milliseconds" after the failure is detected | [K07t] §5.5, pp. 40–41 | `Config::brokenHold` | as written (value: repo choice, below) |
| Ant and data headers | repo choice: [K07t] names the fields (type, source, destination, sequence number, path length) but no layout | One POD header, little-endian, leading version byte (org rule 3): version, type, zone, source, destination, sequence number, hops | repo choice |
| A paper differing from [K07t] on any row | unknown | — | unknown: waits on [KO08] and [KO07] (#24) |

## Parameters

| Parameter | Value | Source | Notes |
|---|---|---|---|
| (ν1, ν2, ν3), initial pheromone by zone | (20, 1, 1) | [K07t] Table 1, p. 42 | secondary: [K07t]. Fig. 18, p. 51 also runs (20,20,1), (20,20,20), (24,18,12); (20,1,1) delivers best |
| μ, evaporation factor per interval | 0.95 | [K07t] Table 1, p. 42, §5.6.1, p. 44 | secondary: [K07t]; Figs. 15–17 try 0.80–0.95 |
| t, standard-deviation threshold | 1 | [K07t] Table 1, p. 42 | secondary: [K07t]; Figs. 12–14 p. 43 try 0.5–2 |
| window size | 50 | [K07t] Table 1, p. 42 | secondary: [K07t] |
| g(d) | 1000/d² | [K07t] Table 1, p. 42 | secondary: [K07t]. Table 1 says "used in equation 5"; it is eq. 3's g |
| n, forward ants per zone per interval | 1 | [K07t] §5.3, p. 35 | secondary: [K07t] |
| (w1, w2, w3), backward-ant weight by zone | (1, 1, 1) | repo choice: [K07t] eq. 4 bounds them (w1 ≥ 1, w2 = 1, w3 ≤ 1) and gives no value | the neutral values inside the bounds, until a paper gives them |
| c, route-length tolerance between zones | 2 hops | repo choice: [K07t] §5.4, p. 39 names it, Table 1 gives no value | unknown in the sources |
| Trail idle timeout | 5 s | repo choice: [K07t] p. 33 says "in the order of seconds" | |
| Broken-mode hold | 50 ms | repo choice: [K07t] pp. 40–41 says "in the order of milliseconds" | |
| Interval (tick) | 100 ms | repo choice: [K07t] counts "clock" steps with no duration (Fig. 12, p. 43) | the adapter's timer period |
| BR table and duplicate-history size | 1024 entries | repo choice: org rule 7 keeps bounded structures bounded | |
| Source data queue | 64 packets | repo choice: org rule 7 | |

## Anchor candidate

| Trend | Scenario | Metric | Comparators | Source |
|---|---|---|---|---|
| POSANT(20,1,1) reaches ~100 % delivery, and GPSR stays lower, with irregular transmission ranges | 80 nodes uniform in a 500 × 500 square, range 50–70 units per direction, connected graphs, one random pair, 200 networks | delivery rate over clock steps | AntNet, AntHocNet, GPSR | [K07t] §5.6.2, p. 45, Fig. 18, p. 51 |
| POSANT(20,1,1)'s average packet delay reaches its minimum faster than AntNet's and the other POSANT settings | as above | average delay (hops) over clock steps | AntNet, AntHocNet, GPSR | [K07t] §5.6.3, p. 46, Fig. 19, p. 51 |
| AntHocNet reaches 100 % delivery first (it broadcasts ants) | as above | delivery rate | AntHocNet | [K07t] §5.6.2, p. 45 |

[K07t]'s simulator is its own (VC++, static graphs, no MAC; §5.6, p. 41), so
these trends are the anchor's shape, not its numbers. Whether the papers ran
the same evaluation waits on #24.

## Rows that wait on #24

Every `secondary: [K07t]` row above needs the `paper` basis from [KO08]
(and [KO07] for deviations), and these need a value or a rule from them:
(w1, w2, w3); c; the trail idle timeout; the broken-mode hold; a neighbour
that appears after trails exist; data before convergence; the d = 0 case;
the header layout; and the evaluation behind the anchor.
