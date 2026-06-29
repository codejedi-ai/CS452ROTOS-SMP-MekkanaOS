# syntax=docker/dockerfile:1
#
# MEKKANA production image — toolchain + QEMU from cs452rotos-platform.
ARG DEV_IMAGE=codejedi-ai/cs452rotos-platform:latest
FROM ${DEV_IMAGE}

ENV XDIR=/opt/toolchain
ENV PATH=/opt/toolchain/bin:${PATH}
WORKDIR /work

COPY . /work
RUN make MODE=qemu && ls -lh 0-d273liu.img

ARG TTYD_VERSION=1.7.7
RUN set -eux; \
    arch=$(uname -m); \
    case "$arch" in \
        x86_64)  asset=ttyd.x86_64 ;; \
        aarch64) asset=ttyd.aarch64 ;; \
        armv7l)  asset=ttyd.armhf ;; \
        *) echo "unsupported ttyd host arch: $arch" >&2; exit 1 ;; \
    esac; \
    curl -fL "https://github.com/tsl0922/ttyd/releases/download/${TTYD_VERSION}/${asset}" \
         -o /usr/local/bin/ttyd; \
    chmod +x /usr/local/bin/ttyd; \
    /usr/local/bin/ttyd --version

RUN apt-get update && apt-get install -y --no-install-recommends \
        git nodejs npm \
    && rm -rf /var/lib/apt/lists/*

ARG MARKLINSIM_REF=master
COPY marklinsim-web /opt/marklinsim-web
RUN set -eux; \
    git clone --depth 1 --branch "${MARKLINSIM_REF}" \
        https://github.com/Martin1994/MarklinSim /opt/MarklinSim; \
    /opt/marklinsim-web/install-overlay.sh /opt/MarklinSim /opt/marklinsim-web

COPY tools/start-prod.sh /usr/local/bin/start-prod.sh
RUN chmod +x /usr/local/bin/start-prod.sh

EXPOSE 8080
CMD ["/usr/local/bin/start-prod.sh"]
