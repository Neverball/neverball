#FROM emscripten/emsdk:3.0.0
FROM emscripten/emsdk

ENV GL4ES_COMMIT=57240b0f06551a49c94f90abccc54767c738791e
ENV GL4ES_DIR=/opt/gl4es

RUN \
    export DEBIAN_FRONTEND=noninteractive; \
    apt-get update -y; \
    apt-get install less

COPY ./install-gl4es.sh /tmp/install-gl4es
RUN \
    chmod +x /tmp/install-gl4es && \
    /tmp/install-gl4es && \
    rm -rf /tmp/install-gl4es

# Use a pre-made emscripten user (UID=1000 GUID=1000) to avoid root-owned files.

RUN echo emscripten ALL=\(root\) NOPASSWD:ALL > /etc/sudoers.d/emscripten && \
    chmod 0440 /etc/sudoers.d/emscripten

USER emscripten