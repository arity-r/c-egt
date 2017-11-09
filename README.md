# c-egt
C implementation of Evolutionary Game Theory

### Requirements
* igraph (C library)
* python
* R (optional)

### Building
To build essential program, (graphgen.out and agos-common.out)
```
make all
```

### Experiment

 1. On `agos-common.py`, Call `set_graph` to initialize network.
 ```python
 set_graph('rrg') # valid parameter: 'rrg', 'sl', and 'ba'
 ```

 2. Call `run_experiment` with some parameters.
 ```python
 #              benefit beta mutation
 run_experiment(1.005,  10,  0)
 ```

 3. Run `agos-common.py`
 ```
 ./agos-common.py
 ``` 

 4. After the script ends, result file(s) is created with following name.
 ```
 benefit=<benefit>;beta=<beta>;mutation=<mutation>;topology=<topology>.<timestamp>.csv
 ```
 For example, `benefit=1.005;beta=10;mutation=0;topology=rrg.20170217183418.csv`.

### Result Format
The following is some lines in a result file.
```
0.0,3854059,0.0
0.001,38326,-38.3232898727
0.002,16802,-30.0655923469
0.003,20774,-43.715723046
0.004,23997,-26.1006227787
0.005,27503,-25.9698946342
0.006,25853,-29.4667963116
0.007,27745,-14.5929477062
0.008,26104,-36.3898207664
0.009,30619,-30.5260783731
```
First column means fraction of cooperation.
Second column means the number of time steps the simulation indicated corresponding FoC
Third column means sum of time-dependent AGoS.

### Data Analysis
`agos-plot.R` provides two functions (`as.nvparams` and `make.result`).
To load result in `result` directory, run following commands.

```r
result.data <- data.frame()
for(mutation in c(0, 1e-5, 1e-4, 0.001, 0.01)) {
  # make a named vector as a set of parameters
  nv <- c(benefit=1.005, beta=10, mutation=mutation, topology='rrg')
  # sort by name
  nv <- nv[order(names(nv))]
  # calculate result for given parameters and append to result.data
  result.data <- rbind(result.data, make.result(nv))
}
```

Then, run following to plot result.

```r
ggplot(result.data, aes(j, G, colour=mutation)) + geom_line()
```

### FoC with Time
`time-evolution.py` calculates FoC for each time step and save to `time-evolution.csv`.
The program runs on following settings.
* All topologies (RRG, SL, and BA)
* Two initial FoCs (0.2 and 0.8)
* Two mutation probabilities (0 and 0.01)
* Two payoff functions (accumulated and average)
* 1000000 time steps per a sim.
* 1000 simulations

Columns of `time-evolution.csv` are
1. Network topology
2. Payoff function ('acc'umulated or 'avr'age)
3. Temptation to defect
4. Intensity of selection
5. Mutation probability
6. Initial FoC
7. Simulation number (1-1000)
8. Timestep
9. FoC on timestep
