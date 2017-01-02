# Subr2009Edge
The implementation of Edge-preserving multiscale image decomposition based on local extrema. Subr, Kartic, Cyril Soler, and Frédo Durand. ACM Transactions on Graphics (TOG) 28.5 (2009): 147.

**File structure:** <br />
Subr2009Edge/ <br />
&nbsp;&nbsp;&nbsp;&nbsp;bin/ <br /> 
&nbsp;&nbsp;&nbsp;&nbsp;data/ <br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;input/ <br />
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;output/ <br />
&nbsp;&nbsp;&nbsp;&nbsp;ref/ <br />
&nbsp;&nbsp;&nbsp;&nbsp;src/ <br />
  
Before run the code,  <br />
1) put input images in `data/input`, and edit the input path in `run.sh`  <br />
2) you should have opencv installed <br />
3) you should have Matlab installed, and add its path to `$PATH`  <br />
(for me, it's like `export PATH=$PATH:/usr/local/MATLAB/R2014a/bin`)  <br />
  
To run the code, first `make` to compile the EdgePreservedBlur.cpp file.  <br />
Then, simply `sh run.sh` to run the program.  <br />

FYI, I also wrote a program in pure c++ (that is, not like the cpp-matlab mixed way here),  <br />
and I used Eigen for solving Ax=b.  <br />
However, the performance is much worse than pure matlab codes!  <br />
(I'll report the running time later)  <br />
So, my final decision is to convert between c++ and matlab.  <br />
Let c++ handle the for loops and have metlab responsible for solving the linear system :)
