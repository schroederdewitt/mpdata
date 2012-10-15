//listing17
#include "listings.hpp"
#define GNUPLOT_ENABLE_BLITZ
#include <gnuplot-iostream/gnuplot-iostream.h>
enum {x, y};

template <class T>
void setup(T &solver, int n[2], float C[2]) {
  blitz::firstIndex i;
  blitz::secondIndex j;
  solver.state() = exp(
    -sqr(i-n[x]/2.) / (2.*pow(n[x]/10, 2))
    -sqr(j-n[y]/2.) / (2.*pow(n[y]/10, 2))
  );  
  solver.Cx() = C[x]; 
  solver.Cy() = C[y];
}

int main() {
  int n[] = {24, 24}, nt = 96;
  float C[] = {.5, .25};
  Gnuplot gp;
  gp << "set term pdf size 10cm, 30cm\n" 
     << "set output 'figure.pdf'\n"     
     << "set multiplot layout 4,1\n" 
     << "set border 4095\n"
     << "set xtics out\n"
     << "set ytics out\n"
     << "unset ztics\n"    
     << "set xlabel 'X'\n"
     << "set ylabel 'Y'\n"
     << "set xrange [0:" << n[x]-1 << "]\n"   
     << "set yrange [0:" << n[y]-1 << "]\n"   
     << "set zrange [-.666:1]\n"   
     << "set cbrange [-.025:1.025]\n"     
     << "set palette maxcolors 42\n"
     << "set pm3d at b\n";
  std::string binfmt;
  {
    solver_2D<mpdata<1>, 
      cyclic<x>, cyclic<y>> solver(n[x],n[y]);
    setup(solver, n, C);
    binfmt = gp.binfmt(solver.state());
    gp << "set title 't=0'\n"
       << "splot '-' binary" << binfmt
       << "with lines notitle\n";
    gp.sendBinary(solver.state().copy());
    solver.solve(nt);
    gp << "set title 'donorcell t="<<nt<<"'\n"
       << "splot '-' binary" << binfmt
       << "with lines notitle\n";
    gp.sendBinary(solver.state().copy());
  } {
    const int it = 2;
    solver_2D<mpdata<it>, 
      cyclic<x>, cyclic<y>> solver(n[x],n[y]); 
    setup(solver, n, C); 
    solver.solve(nt);
    gp << "set title 'mpdata<" << it << "> "
       << "t=" << nt << "'\n"
       << "splot '-' binary" << binfmt
       << "with lines notitle\n";
    gp.sendBinary(solver.state().copy());
  } {
    const int it = 4;
    solver_2D<mpdata<it>, 
      cyclic<x>, cyclic<y>> solver(n[x],n[y]); 
    setup(solver, n, C); 
    solver.solve(nt); 
    gp << "set title 'mpdata<" << it << "> "
       << "t=" << nt << "'\n"
       << "splot '-' binary" << binfmt
       << "with lines notitle\n";
    gp.sendBinary(solver.state().copy());
  }
}
//listing18