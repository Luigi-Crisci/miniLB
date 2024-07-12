#include <bcond_driven.hh>
#include <time.hh>

namespace bgk {

void bcond_driven(storage &bgk_storage) {
  // Lid-driven cavity boundary condition
  real_kinds::mystorage force;
  auto &timing = utils::timing::instance();
  auto &q = *bgk_storage.dev_q;

  utils::system_clock(timing.countA0, timing.count_rate, timing.count_max);
  utils::time(timing.tcountA0);

  force = bgk_storage.u00 / real_kinds::mykind{6.0};

#ifdef TRICK1
  // it is correct only if l=m
  // clang-format off
    q.parallel_for(sycl::range(bgk_storage.m), [
        a01 = bgk_storage.a01_device,
        a03 = bgk_storage.a03_device,
        a05 = bgk_storage.a05_device,
        a08 = bgk_storage.a08_device,
        a10 = bgk_storage.a10_device,
        a12 = bgk_storage.a12_device,
        a14 = bgk_storage.a14_device,
        a17 = bgk_storage.a17_device,
        force,
        l1 = bgk_storage.l1,
        m1 = bgk_storage.m1,
        l = bgk_storage.l,
        m = bgk_storage.m
    ](sycl::item<1> id){
        const auto j = id.get_linear_id() + 1;
        a01(j - 1,m1) = a12(j,m) + force;
        a17(j,m1) = a08(j,m);
        a10(j + 1,m1) = a03(j,m) - force;

        // front (x = l)
        a12(l1,j - 1) = a01(l,j);
        a10(l1,j + 1) = a03(l,j);
        a14(l1,j) = a05(l,j);

        // rear (x = 0)
        a03(0,j - 1) = a10(1,j);
        a01(0,j + 1) = a12(1,j);
        a05(0,j) = a14(1,j);

        // left (y = 0)
        a03(j - 1,0) = a10(j,1);
        a08(j,0) = a17(j,1);
        a12(j + 1,0) = a01(j,1);
    }).wait_and_throw();
    //clang-format on
#else

// clang-format off
q.parallel_for(sycl::range(bgk_storage.m), 
[
    a01 = bgk_storage.a01_device,
    a03 = bgk_storage.a03_device,
    a05 = bgk_storage.a05_device,
    a10 = bgk_storage.a10_device,
    a12 = bgk_storage.a12_device,
    a14 = bgk_storage.a14_device,
    l1 = bgk_storage.l1,
    l = bgk_storage.l
](sycl::item<1> id){
    const auto j = id.get_linear_id() + 1;
    // front (x = l)
    a12(l1,j-1) = a01(l,j);
    a10(l1,j+1) = a03(l,j);
    a14(l1,j) = a05(l,j);

    // rear (x = 0)
    a03(0,j-1) = a10(1,j);
    a01(0,j+1) = a12(1,j);
    a05(0,j) = a14(1,j);
}).wait_and_throw();
//clang-format on

#endif

    utils::system_clock(timing.countA1, timing.count_rate, timing.count_max);
    utils::time(timing.tcountA1);
    timing.time_bc = timing.time_bc + static_cast<float>((timing.countA1-timing.countA0)/(timing.count_rate));
    timing.time_bc1 = timing.time_bc1 + (timing.tcountA1-timing.tcountA0);

    #ifdef DEBUG_2
        if(bgk_storage.myrank == 0) {
           std::cout << "DEBUG2: Exiting from sub. bcond_driven";
        }
#endif

}
} // namespace bgk