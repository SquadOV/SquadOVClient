FROM debian:buster-20200908-slim AS builder
ARG GCP_PROJECT
RUN apt-get update && apt-get install -y --no-install-recommends curl \
    ca-certificates \
    build-essential \
    gnupg2 \
    && rm -rf /var/lib/apt/lists/*
RUN curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add -
RUN echo "deb https://dl.yarnpkg.com/debian/ stable main" | tee /etc/apt/sources.list.d/yarn.list
RUN apt-get update && apt-get -y install yarn && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /squadov
COPY layouts /squadov/layouts
COPY pages /squadov/pages
COPY plugins /squadov/plugins
COPY static /squadov/static
COPY package.json /squadov
COPY shims.d.ts /squadov
COPY tsconfig.json /squadov
COPY nuxt.config.js /squadov
COPY yarn.lock /squadov

WORKDIR /squadov
RUN yarn install && yarn generate

FROM nginx:1.19.3
RUN yes | rm /etc/nginx/conf.d/*
RUN apt-get update && apt-get install -y cron curl
RUN curl https://ssl-config.mozilla.org/ffdhe2048.txt > /etc/nginx/conf.d/dhparam.pem
COPY nginx.conf /etc/nginx/conf.d/nginx.conf
RUN mkdir -p /squadov
COPY --from=builder /squadov/docs /squadov/docs
EXPOSE 8080/tcp