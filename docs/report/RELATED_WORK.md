# Related work (draft, Eimi)

> Draft of the *Related work* section. Numbers are quoted from the cited papers; where a number comes from the body of a paper and not from its abstract, the section is given.

## 2.1 Floorplan representations and search

Most floorplanners encode a floorplan in a data structure that can be perturbed and packed quickly, and search over it with simulated annealing (SA). Wong and Liu used normalized Polish expressions, which only represent *slicing* floorplans, and optimized area and wirelength together with SA [1]. The sequence pair of Murata et al. encodes any packing as two orderings of the modules [2]. The O-tree [3] and the B\*-tree [4] are ordered trees for non-slicing, bottom-left compacted floorplans; a B\*-tree is packed with a contour, and its moves (rotate, swap, move a node) keep the tree valid without extra checks. Chang et al. reported 1.27 mm² for ami33 and 36.80 mm² for ami49 with B\*-tree SA, optimizing area alone (Table 2 of [4]). Chen and Chang added a three-stage temperature schedule, Fast-SA, and an adaptive weighting for fixed-outline floorplanning [5].

Adya and Markov showed that fixed-outline instances, where the chip must fit a given rectangle, are much harder than the classical problem, and implemented new SA objectives and moves on top of the sequence pair in the Parquet floorplanner [6]. Fixed outline is the setting of hierarchical design flows and the one we use: the MCNC files give an outline, and for GSRC we build one with 15 % whitespace and aspect ratio 1, as is common in this literature.

ThermoPlace uses the B\*-tree of [4] with the classical SA schedule (geometric cooling) and adds moves of whole subtrees. Fast-SA [5] would likely reduce runtime and is left as future work.

## 2.2 Thermal models

HotSpot models the chip, the heat spreader and the heat sink as an equivalent RC circuit, and was validated against finite-element simulation [7], [8]. It works per block or on a grid, and is the reference simulator of most academic thermal-aware floorplanners. 3D-ICE extends compact modeling to 3D stacks with liquid cooling and reports a speed-up of "up to 975x" against a commercial CFD tool with a maximum temperature error of 3.4 % [9].

Inside an optimizer even HotSpot is slow, so floorplanners use faster approximations and check them against it. Xiao et al. proposed an interpolation model for fixed-outline 3D floorplanning and report a correlation of 0.97 with the HotSpot-based model over 30 floorplans (Section II of [10]). RLPlanner, a reinforcement-learning floorplanner for chiplets, uses a fast thermal model with a mean absolute error of 0.25 K and "over 120x speed-up" against HotSpot [11]. These works report how close the fast model is to the reference on average; they do not report how the error changes with the resolution of the model, which is the question we study for a 2D grid.

## 2.3 Thermal-aware floorplanning

Sankaranarayanan et al. argued for thermal-aware floorplanning at the microarchitectural level and presented HotFloorplan, an SA floorplanner that trades temperature against wire delay [12]. Hung et al. used a genetic algorithm with HotSpot as the thermal evaluator and obtained lower peak temperatures with area similar to area-only floorplanning on MCNC benchmarks and a face-detection chip [13]. Han and Koren studied SA-based temperature-aware floorplanning [14]. For 3D ICs, Cong et al. combined a 3D floorplan representation with a compact resistive thermal model and report a 56 % lower maximum temperature than a 3D floorplanner that ignores temperature, and 29 % less wirelength than a previous 3D method [15].

The problem now reappears at the package level, with chiplets on an interposer. TAP-2.5D inserts spacing between chiplets to reduce temperature and wirelength together; in its multi-GPU case it lowers the operating temperature by 4 °C at the cost of 10 % more wirelength (body of [16]). RLPlanner reports an average improvement of 20.28 % of its objective over SA with HotSpot [11].

**Where ThermoPlace fits.** Like [12]–[15], we add the peak temperature to the cost of the search. We differ in three points: the thermal model is a plain 2D grid whose only free parameters (two package values) are calibrated against HotSpot and published together with the code; we measure its error against HotSpot for several grid sizes; and we report the area cost of the thermal term for two power ranges, which shows how much of the peak temperature the floorplan can change at all.

## References (numbering local to this draft)

1. D. F. Wong and C. L. Liu, "A new algorithm for floorplan design," in *Proc. 23rd ACM/IEEE Design Automation Conf. (DAC)*, 1986, pp. 101–107, doi:10.1145/318013.318030.
2. H. Murata, K. Fujiyoshi, S. Nakatake and Y. Kajitani, "VLSI module placement based on rectangle-packing by the sequence-pair," *IEEE Trans. Computer-Aided Design*, vol. 15, no. 12, pp. 1518–1524, Dec. 1996, doi:10.1109/43.552084.
3. P.-N. Guo, C.-K. Cheng and T. Yoshimura, "An O-tree representation of non-slicing floorplan and its applications," in *Proc. 36th DAC*, 1999, pp. 268–273, doi:10.1145/309847.309928.
4. Y.-C. Chang, Y.-W. Chang, G.-M. Wu and S.-W. Wu, "B\*-Trees: A new representation for non-slicing floorplans," in *Proc. 37th DAC*, 2000, pp. 458–463, doi:10.1109/DAC.2000.855354.
5. T.-C. Chen and Y.-W. Chang, "Modern floorplanning based on B\*-tree and fast simulated annealing," *IEEE Trans. Computer-Aided Design*, vol. 25, no. 4, pp. 637–650, Apr. 2006.
6. S. N. Adya and I. L. Markov, "Fixed-outline floorplanning: enabling hierarchical design," *IEEE Trans. VLSI Syst.*, vol. 11, no. 6, pp. 1120–1135, Dec. 2003, doi:10.1109/TVLSI.2003.817546.
7. K. Skadron, M. R. Stan, W. Huang, S. Velusamy, K. Sankaranarayanan and D. Tarjan, "Temperature-aware microarchitecture," in *Proc. 30th ISCA*, 2003, pp. 2–13, doi:10.1109/ISCA.2003.1206984.
8. W. Huang, S. Ghosh, S. Velusamy, K. Sankaranarayanan, K. Skadron and M. R. Stan, "HotSpot: A compact thermal modeling methodology for early-stage VLSI design," *IEEE Trans. VLSI Syst.*, vol. 14, no. 5, pp. 501–513, May 2006.
9. A. Sridhar, A. Vincenzi, M. Ruggiero, T. Brunschwiler and D. Atienza, "3D-ICE: Fast compact transient thermal modeling for 3D ICs with inter-tier liquid cooling," in *Proc. ICCAD*, 2010, pp. 463–470, doi:10.1109/ICCAD.2010.5653749.
10. L. Xiao, S. Sinha, J. Xu and E. F. Y. Young, "Fixed-outline thermal-aware 3D floorplanning," in *Proc. 15th Asia and South Pacific Design Automation Conf. (ASP-DAC)*, 2010, pp. 561–567, doi:10.1109/ASPDAC.2010.5419822.
11. Y. Duan, X. Liu, Z. Yu, H. Wu, L. Shao and X. Zhu, "RLPlanner: Reinforcement learning based floorplanning for chiplets with fast thermal analysis," arXiv:2312.16895, 2023.
12. K. Sankaranarayanan, S. Velusamy, M. Stan and K. Skadron, "A case for thermal-aware floorplanning at the microarchitectural level," *J. Instruction-Level Parallelism*, vol. 7, 2005. [CHECK volume and pages]
13. W.-L. Hung, Y. Xie, N. Vijaykrishnan, C. Addo-Quaye, T. Theocharides and M. J. Irwin, "Thermal-aware floorplanning using genetic algorithms," in *Proc. 6th Int. Symp. Quality Electronic Design (ISQED)*, 2005, pp. 634–639, doi:10.1109/ISQED.2005.122.
14. Y. Han and I. Koren, "Simulated annealing based temperature aware floorplanning," *J. Low Power Electronics*, vol. 3, no. 2, pp. 141–155, 2007, doi:10.1166/jolpe.2007.128.
15. J. Cong, J. Wei and Y. Zhang, "A thermal-driven floorplanning algorithm for 3D ICs," in *Proc. ICCAD*, 2004, pp. 306–313, doi:10.1109/ICCAD.2004.1382591.
16. Y. Ma, L. Delshadtehrani, C. Demirkiran, J. L. Abellán and A. Joshi, "TAP-2.5D: A thermally-aware chiplet placement methodology for 2.5D systems," in *Proc. DATE*, 2021, doi:10.23919/DATE51398.2021.9474011.
