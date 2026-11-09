#include "agent.h"
#include "file_source.h"
#include "os_source.h"

#include <charconv>
#include <cstddef>
#include <cstdio>
#include <exception>
#include <print>
#include <string>
#include <system_error>

namespace {

struct Arguments {
    bool quiet = false;
    bool use_file = false;
    bool path_set = false;
    std::string path;
};

bool ParseArgs(int argc, char** argv, Arguments& args) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        if (arg == "--quiet") {
            args.quiet = true;
        } else if (arg == "--file") {
            if (args.path_set) {
                std::print(stderr, "путь указан несколько раз\n");
                return false;
            }
            if (i + 1 >= argc) {
                std::print(stderr, "после --file требуется путь к журналу\n");
                return false;
            }
            args.path = argv[++i];
            args.path_set = true;
            args.use_file = true;
        } else {
            if (args.path_set) {
                if (arg[0] != '-') {
                    std::print(stderr, "путь указан несколько раз\n");
                } else {
                    std::print(stderr, "неизвестная опция: {}\n", arg);
                }
                return false;
            }
            args.path = arg;
            args.path_set = true;
        }
    }

    if (!args.path_set || args.path.empty()) {
        std::print(stderr,
                   "использование: nano-edr [--quiet] <сценарий>\n"
                   "или: nano-edr [--quiet] --file <журнал>\n");
        return false;
    }

    return true;
}

}  // namespace

int main(int argc, char** argv) {
    Arguments args;
    if (!ParseArgs(argc, argv, args)) return 2;

    try {
        nano_edr::Agent agent(args.quiet);
        std::unique_ptr<nano_edr::IEventSource> source;
        if (args.use_file) {
            source = std::make_unique<nano_edr::FileSource>(args.path);
        } else {
            source = std::make_unique<nano_edr::OsSource>(args.path);
        }
        source->Run(&agent);
        agent.PrintSummary();
        return 0;
    } catch (const std::exception& error) {
        std::print(stderr, "произошла ошибка: {}\n", error.what());
        return 1;
    }
}
