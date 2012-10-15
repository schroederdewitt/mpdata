//listing00
// code licensed under the terms of GNU GPL v3
// copyright holder: University of Warsaw
//listing01
#include <blitz/array.h>
using arr_t = blitz::Array<double, 2>;
using rng_t = blitz::Range;
using idx_t = blitz::RectDomain<2>;
//listing02
#define return_macro(expr) \
  -> decltype(safeToReturn(expr)) \
{ return safeToReturn(expr); } 
//listing03
#include <boost/ptr_container/ptr_vector.hpp>
struct arrvec_t : boost::ptr_vector<arr_t> {
  const arr_t &operator[](const int i) const {   
    return this->at(
      (i + this->size()) % this->size()
    ); 
  }
};
//listing04
struct hlf_t {} h;

inline rng_t operator+(
  const rng_t &i, const hlf_t &
) { 
  return i + 1; 
} 

inline rng_t operator-(
  const rng_t &i, const hlf_t &
) { 
  return i; 
}
//listing05
template<class n_t>
inline rng_t operator^(
  const rng_t &r, const n_t &n
) { 
  return rng_t(
    (r - n).first(), 
    (r + n).last()
  ); 
} 
//listing06
template<class adv_t, class bcx_t, class bcy_t>
struct solver_2D
{
  // member fields
  arrvec_t psi, C;
  int n;
  rng_t i, j;
  adv_t adv;
  bcx_t bcx;
  bcy_t bcy;

  // ctor
  solver_2D(int nx, int ny) :
    n(0), i(0, nx-1), j(0, ny-1), adv(i, j), 
    bcx(i, j, adv_t::n_halos), 
    bcy(j, i, adv_t::n_halos)
  {
    const int hlo = adv.n_halos;
    for (int l = 0; l < 2; ++l) 
      psi.push_back(new arr_t(i^hlo, j^hlo));
    // +/-hlo (Blitz requires equal lower bounds)
    C.push_back(new arr_t(i^h^hlo, j^hlo));
    C.push_back(new arr_t(i^hlo, j^h^hlo));
  }

  // accessor methods
  arr_t state() {
    return psi[n](i,j).reindex({0,0});
  }
  arr_t Cx() { return C[0]; } 
  arr_t Cy() { return C[1]; } 

  // integration logic
  void solve(const int nt) {
    for (int t = 0; t < nt; ++t) {
      for (int s = 0; s < adv.n_steps; ++s) {
        bcx.fill_halos(psi[n]);
        bcy.fill_halos(psi[n]);
        adv.op_2D(psi, n, C, i, j, s);
        n = (n + 1) % 2;
      }
    }
  }
};
//listing07
template<int d> 
struct pi;

template<>
struct pi<0> : idx_t { 
  pi(const rng_t &i, const rng_t &j) :
    idx_t({i,j})
  {}  
};

template<>
struct pi<1> : idx_t { 
  pi(const rng_t &j, const rng_t &i) :
    idx_t({i,j}) 
  {}  
}; 
//listing08
template<int d>
struct cyclic
{
  // member fields
  pi<d> left_halo, rght_halo;
  pi<d> left_edge, rght_edge;;

  // ctor
  cyclic(
    const rng_t &i, const rng_t &j, int hlo
  ) :
    left_halo(
      rng_t(i.first()-hlo, i.first()-1), j^hlo
    ),
    rght_edge(
      rng_t(i.last()-hlo+1, i.last()  ), j^hlo
    ),
    rght_halo(
      rng_t(i.last()+1, i.last()+hlo  ), j^hlo
    ),
    left_edge(
      rng_t(i.first(), i.first()+hlo-1), j^hlo
    )
  {} 

  // method invoked by the solver
  void fill_halos(const arr_t &psi)
  {
    psi(left_halo) = psi(rght_edge);     
    psi(rght_halo) = psi(left_edge);     
  }
};
//listing09
template<class T1, class T2, class T3> 
inline auto F(
  const T1 &psi_l, const T2 &psi_r, const T3 &C
) return_macro(
  .5 * (C + abs(C)) * psi_l + 
  .5 * (C - abs(C)) * psi_r
)
//listing10
template<int d>  
inline auto donorcell( 
  const arr_t &psi, const arr_t &C, 
  const rng_t &i, const rng_t &j
) return_macro(
  F(
    psi(pi<d>(i,   j)), 
    psi(pi<d>(i+1, j)), 
      C(pi<d>(i+h, j))
  ) -
  F(
    psi(pi<d>(i-1, j)), 
    psi(pi<d>(i,   j)), 
      C(pi<d>(i-h, j))
  )
)
//listing11
inline void donorcell_2D(
  const arrvec_t &psi, const int n,
  const arrvec_t &C, 
  const rng_t &i, const rng_t &j
) { 
  psi[n+1](i,j) = psi[n](i,j)
    - donorcell<0>(psi[n], C[0], i, j)
    - donorcell<1>(psi[n], C[1], j, i); 
}
//listing12
template<class nom_t, class den_t>
static inline auto frac(
  const nom_t &nom, const den_t &den
) return_macro(
  where(den > 0, nom / den, 0)
) 
//listing13
template<int d>
inline auto A(const arr_t &psi, 
  const rng_t &i, const rng_t &j
) return_macro(
  frac(
    psi(pi<d>(i+1, j)) - psi(pi<d>(i,j)),
    psi(pi<d>(i+1, j)) + psi(pi<d>(i,j))
  ) 
) 
//listing14
template<int d>
inline auto B(const arr_t &psi, 
  const rng_t &i, const rng_t &j
) return_macro(
 .5 * frac(
    psi(pi<d>(i+1, j+1)) + psi(pi<d>(i, j+1)) -
    psi(pi<d>(i+1, j-1)) - psi(pi<d>(i, j-1)),
    psi(pi<d>(i+1, j+1)) + psi(pi<d>(i, j+1)) +
    psi(pi<d>(i+1, j-1)) + psi(pi<d>(i, j-1))
  )
)
//listing15
template<int d>
inline auto antidiff_2D(
  const arr_t &psi, 
  const rng_t &i, const rng_t &j,
  const arrvec_t &C
) return_macro(
  abs(C[d](pi<d>(i+h, j))) 
  * (1 - abs(C[d](pi<d>(i+h, j)))) 
  * A<d>(psi, i, j) 
  - C[d](pi<d>(i+h, j)) 
  * .25 * (
    C[d-1](pi<d>(i+1, j+h)) + 
    C[d-1](pi<d>(i,   j+h)) +
    C[d-1](pi<d>(i+1, j-h)) + 
    C[d-1](pi<d>(i,   j-h)) 
  ) 
  * B<d>(psi, i, j)
) 
//listing16
template<int n_iters>
struct mpdata 
{
  // member fields
  arrvec_t tmp0, tmp1;
  static const int n_steps = n_iters;
  static const int n_halos = n_iters;

  // ctor
  mpdata(const rng_t &i, const rng_t &j) {
    const int hlo = n_halos;
    tmp0.push_back(new arr_t(i^h^hlo, j^hlo));
    tmp0.push_back(new arr_t(i^hlo, j^h^hlo));
    if (n_iters < 2) return;
    tmp1.push_back(new arr_t(i^h^hlo, j^hlo));
    tmp1.push_back(new arr_t(i^hlo, j^h^hlo));
  }

  // method invoked by the solver
  inline void op_2D( 
    const arrvec_t &psi, const int n, 
    const arrvec_t &C, 
    const rng_t &i, const rng_t &j, 
    const int step
  ) {
    // chosing input/output for antidiff. C
    const arrvec_t 
      &C_unco = (step == 1) 
        ? C 
        : (step % 2) 
          ? tmp0 
          : tmp1,
      &C_corr = (step  % 2) 
        ? tmp1 
        : tmp0;

    // calculating the antidiffusive velocities
    if (step > 0) {
      const int hlo = n_steps - 1 - step;
      // extended ranges as we only use C_+1/2 
      const rng_t 
        im(i.first() - 1, i.last()),
        jm(j.first() - 1, j.last());
      C_corr[0](im+h, j^hlo) = antidiff_2D<0>(
        psi[n], im, j^hlo, C_unco
      );
      C_corr[1](i^hlo, jm+h) = antidiff_2D<1>(
        psi[n], jm, i ^ hlo, C_unco
      );
    }

    // donor-cell step with C or C_corr
    if (step == 0) 
      donorcell_2D(psi, n, C,      i, j);
    else    
      donorcell_2D(psi, n, C_corr, i, j);
  }
};
//listing17