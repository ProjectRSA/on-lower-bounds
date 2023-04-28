Before starting, the reporting in the folder Files is the first theoritical base for these files, you should read it.
First verify if the CPLEX path in the makefile is the same of the CPLEX location in your directory

1 - compile the code with the comand "make"
2 - execute with ./exe INSTANCE_FOLDER INSTANCE_FOLDER/output.txt

If you want to save all the output from terminal to a separate file (to make a report for example) 
just go to RSA_Solver.cpp file and remove the comentary from the line "freopen(argv[2], "w", stdout);" 
this will save all output that should appear in the terminal to a file located in the INSTANCE_FOLDER with the name output.txt 	

I call this program "The Pathfinder" (Mass Effect fans will understand)
