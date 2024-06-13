#include <she_test.h>

int main(int argc, char** argv) {
  she_test::details::register_center<>::init_and_run(argc, argv);

  return 0;
}
