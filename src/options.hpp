#include <cxxopts.hpp>

class Option {
 public:
  const bool interactive_mode_;
  const bool enable_interpreter_;
  const bool enable_json_tree_;
  const bool enable_llvm_ir_;

  Option()
      : interactive_mode_(true),
        enable_interpreter_(true),
        enable_json_tree_(true),
        enable_llvm_ir_(true) {}

  Option(bool interactive_mode, bool enable_interpreter, bool enable_json_tree,
         bool enable_llvm_ir)
      : interactive_mode_(interactive_mode),
        enable_interpreter_(enable_interpreter),
        enable_json_tree_(enable_json_tree),
        enable_llvm_ir_(enable_llvm_ir) {}

  ~Option() {}

  static Option* parse(int argc, char* argv[]) {
    cxxopts::Options cxx_options("BLC", "Basic Calculator with LLVM.");

    bool interactive_mode, enable_interpreter, enable_json_tree, enable_llvm_ir;

    cxx_options.add_options()(
        "interactive", "Interactive mode that respond user input immediately.",
        cxxopts::value<bool>(interactive_mode)->default_value("true"))(
        "interpreter", "Enable interpreter.",
        cxxopts::value<bool>(enable_interpreter)->default_value("true"))(
        "tree", "Enable generation of syntax tree in JSON format.",
        cxxopts::value<bool>(enable_json_tree)->default_value("true"))(
        "llvm", "Enable generation of LLVM IR.",
        cxxopts::value<bool>(enable_llvm_ir)->default_value("true"))(
        "h,help", "Display help message.",
        cxxopts::value<bool>()->default_value("false"));

    auto result = cxx_options.parse(argc, argv);

    if (result["help"].as<bool>()) {
      std::cout << cxx_options.help();
      exit(0);
    }

    return new Option(interactive_mode, enable_interpreter, enable_json_tree,
                      enable_llvm_ir);
  }
};