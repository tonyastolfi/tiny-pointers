# syntax=docker/dockerfile:1

FROM registry.gitlab.com/batteriesincluded/batt-docker/batteries-debian12-build-tools:0.4.0

RUN apt-get update && apt-get remove --purge -y cmake

RUN mkdir -p /build
COPY . /build
WORKDIR /build

RUN python3 -m venv /tools-env
RUN . /tools-env/bin/activate && \
    pip install --upgrade pipx && \
    pip install batt-cli --index-url https://gitlab.com/api/v4/projects/64628567/packages/pypi/simple && \
    cor-setup && \
    cor setup-conan && \
    echo "" >> ~/.conan2/profiles/default && \
    echo "[tool_requires]" >> ~/.conan2/profiles/default && \
    echo "cmake/3.31.6" >> ~/.conan2/profiles/default && \
    cat ~/.conan2/profiles/default && \
    cor test --clean
