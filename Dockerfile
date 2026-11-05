# Optional toolchain image. The project itself does not need Docker:
# any Linux, macOS or Windows machine with CMake >= 3.22 and a C++20 compiler builds it
# the same way, with the packages listed in README.md or with the copies in third_party/.
FROM debian:bookworm-slim

RUN apt-get update \
 && apt-get install -y --no-install-recommends \
      g++ cmake ninja-build make git ca-certificates \
      clang-tidy \
      libgtest-dev libgmock-dev libfmt-dev nlohmann-json3-dev \
      python3 \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /work
CMD ["bash"]
