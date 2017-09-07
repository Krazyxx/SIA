#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <random>

#include "gnuplot_i.hpp"

using namespace std;
default_random_engine gener;

double int_mc(double(*)(double), double&, double, double, int);
double f(double);
double var_anal(int);
void wait_for_key ();

int main()
{
  srand(time(NULL));

  Gnuplot g("lines");

  double a = 0.0;              // left endpoint
  double b = 1.0;               // right endpoint
  int N = 500;
    
  double varest, mc, va;
  std::vector<double> x, err, var_va, var_ea, var_vn, var_en;

  for (int n=5; n <= N; n++)
  {
    mc = int_mc(f, varest, a, b, n);
    va = var_anal(n);
    
    x.push_back(n);
    err.push_back(fabs(mc-1.0));
    var_va.push_back(va);        // variance analyptique
    var_ea.push_back(sqrt(va));  // écart-type analyptique

    var_vn.push_back(varest);
    var_en.push_back(sqrt(varest));
  }

  g.reset_plot();
  g.set_grid();
  g.set_style("lines").plot_xy(x,err,"erreur d'estimation");

  //g.set_style("lines").plot_xy(x,var_va,"variance analyptique");
  g.set_style("lines").plot_xy(x,var_ea,"ecart-type analyptique");

  //g.set_style("lines").plot_xy(x,var_vn,"variance numerique");
  g.set_style("lines").plot_xy(x,var_en,"ecart-type numerique");
  
  wait_for_key();

  return 0;
}

/* 
   Fonction à intégrer 
*/
double f(double x)
{
  double y = 5 * x * x * x * x;
  return y;
}    

double var_anal(int n)
{
  return (16./9.)/((double) n);
}

/* 
   Integration de Monte-Carlo
*/
double int_mc(double(*f)(double), double& varest, double a, double b, int n)
{
  uniform_real_distribution<float> distr (a,b);
  
  double res = 0, res2 = 0;;
  for (int i = 0; i < n; i++) {
    double random = distr(gener);
    double value = f(random);
    res += value;
    res2 += value * value;
  }
  res /= n;
  res2 /= n;
  
  varest = (1./(n-1.)) * (res2 - res * res);
    
  return res;
}

void wait_for_key ()
{
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__TOS_WIN__)  // every keypress registered, also arrow keys
  cout << endl << "Press any key to continue..." << endl;

  FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
  _getch();
#elif defined(unix) || defined(__unix) || defined(__unix__) || defined(__APPLE__)
  cout << endl << "Press ENTER to continue..." << endl;

  std::cin.clear();
  std::cin.ignore(std::cin.rdbuf()->in_avail());
  std::cin.get();
#endif
  return;
}
