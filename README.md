# brp_heur
An implementation of the ALNS metaheuristic for the bicycle repositioning problem. This is the algorithm that provided an upper bound to the rebalancing problem in [Technical Report](https://www.cirrelt.ca/documentstravail/cirrelt-2025-02.pdf).

## Building the code in Linux

1. Clone the repository and add executable permission to a script that will call CMake for you:

```shell
git clone https://github.com/lucasparada20/brp_heur.git
cd brp_heur
chmod u+x cmake_script_heur_dhin.sh
```
2. Build the code by typing:

```bash
./cmake_script_heur_dhin.sh
```

if you want to debug or use valgrind, just type:

```bash
./cmake_script_heur_dhin.sh debug
```

```bash
./cmake_script_heur_dhin.sh valgrind
```

## Instances

To test the code you can use the instances from this [article](https://www.sciencedirect.com/science/article/pii/S0305048313001187) which can be found in this [link](http://www.or.unimore.it/site/home/online-resources/bike-sharing-rebalancing-problems/articolo1090035457.html). There should be 65 instances there, as noted in the paper.

## Running the code

Inside the brp_heur directory, you will find a script run_heur_dhin.sh with sample command line calls. The format is:

* instance_file : the instance to solve.
* iterations : the number of iterations to use in ALNS. The instances from Dell'Amico et al (2014), have very few stations so ALNS can run very fast with them. For example, 500k or more iterations should take less than 2 minutes in a reasonable CPU.

Example: 

```bash
build/exec_heur instance_file=instances_dhin/Bari30.txt iterations=750000
```

## Ouput

Upon calling the examples in run_heur_dhin.sh, you should see the following output:

![brp_output](https://github.com/lucasparada20/brp_heur/blob/main/images/brp_output.jpg)

## SLR Instances

In the directory `instances_slr`, you will find the instances of the [Technical Report](https://www.cirrelt.ca/documentstravail/cirrelt-2025-02.pdf). These instances require building the code with the script `cmake_script_heur_slr.sh` by typing:

```bash
chmod u+x cmake_script_heur_slr.sh
./cmake_script_heur_slr.sh
```

Alternatively, inside the directory 'src_heur_slr', you will also find a `Makefile` that can build the code by typing:

```bash
make
```

To call the executable for these instances, the script `run_heur_slr.sh` contains all the shell commands you need. Lastly, this code will store results and output files in a directory named `results`. So, be sure to create it inside the `brp_heur` directory first.


