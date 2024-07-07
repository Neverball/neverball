FROM emscripten/emsdk:3.1.60-arm64

ENV GL4ES_COMMIT=c9895df34cd466c23bc60c2bd3db3d87e98fcbe7
ENV GL4ES_DIR=/opt/gl4es

RUN \
    export DEBIAN_FRONTEND=noninteractive; \
    apt-get update -y; \
    apt-get install less

COPY ./install-gl4es.sh /tmp/install-gl4es
RUN \
    chmod +x /tmp/install-gl4es && \
    /tmp/install-gl4es && \
    chown -R emscripten:emscripten "$GL4ES_DIR" && \
    rm -rf /tmp/install-gl4es

# Use a pre-made emscripten user (UID=1000 GUID=1000) to avoid root-owned files.

RUN echo emscripten ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/emscripten && \
    chmod 0440 /etc/sudoers.d/emscripten

USER emscripten