SERIAL_TARGET_BB=knapsackBB_serial
MPI_TARGET_BB=knapsackBB_mpi
RESULTS=results.csv
RESULTS_SPEEDUPS=results_speedups.csv
RESULTS_TIMES=results_times_per_problem.csv
PLOT_FILENAME=results_times_per_problem.svg
OUTPUT_FOLER=output
ERROR_FOLDER=errors
THREADS_FILE=threads.csv

all : serial mpi

bb : $(SERIAL_TARGET_BB) $(MPI_TARGET_BB)

serial : $(SERIAL_TARGET_BB)

$(SERIAL_TARGET_BB) : knapsackBB_serial.cpp
	g++ knapsackBB_serial.cpp -o $(SERIAL_TARGET_BB)

mpi : $(MPI_TARGET_BB)

$(MPI_TARGET_BB) : knapsackBB_mpi.cpp
	mpic++ knapsackBB_mpi.cpp -o $(MPI_TARGET_BB)

run-local : bb
	./run_all_local.sh

run-moore : bb
	./run_all.sh

results : $(RESULTS) $(RESULTS_TIMES) $(RESULTS_SPEEDUPS)

$(RESULTS) : $(OUTPUT_FOLER)/*
	./get_results.sh > $(RESULTS)

$(RESULTS_TIMES) : $(RESULTS) $(THREADS_FILE)
	./get_times_per_problem.py $(RESULTS) $(THREADS_FILE) > $(RESULTS_TIMES)

$(RESULTS_SPEEDUPS) : $(RESULTS)
	./get_speedups.py $(RESULTS) > $(RESULTS_SPEEDUPS)

$(PLOT_FILENAME) : $(RESULTS_TIMES) $(THREADS_FILE)
	./plot_generator.py $(RESULTS_TIMES) $(THREADS_FILE) $(PLOT_FILENAME)

plot : $(PLOT_FILENAME)

clean : clean-outputs clean-errors
	rm -rf $(SERIAL_TARGET_BB) $(MPI_TARGET_BB) $(RESULTS) $(PLOT_FILENAME) \
		$(RESULTS_SPEEDUPS) $(RESULTS_TIMES) $(THREADS_FILE)

clean-outputs :
	rm -rf $(OUTPUT_FOLER)

clean-errors :
	rm -rf $(ERROR_FOLDER)