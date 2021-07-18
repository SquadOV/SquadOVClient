FROM debian:buster-20200908-slim AS builder
ARG DEPLOYMENT_ENVIRONMENT
RUN apt-get update && apt-get install -y --no-install-recommends curl \
    ca-certificates \
    build-essential \
    gnupg2 \
    && rm -rf /var/lib/apt/lists/*
RUN curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add -
RUN echo "deb https://dl.yarnpkg.com/debian/ stable main" | tee /etc/apt/sources.list.d/yarn.list
RUN apt-get update && apt-get -y install yarn && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /squadov
RUN mkdir -p /squadov/config
COPY assets /squadov/assets
COPY config/${DEPLOYMENT_ENVIRONMENT}.json /squadov/config/config.json
COPY js /squadov/js
COPY ui /squadov/ui
COPY views /squadov/views
COPY webpack /squadov/webpack
COPY index.js /squadov
COPY package.json /squadov
COPY tsconfig.json /squadov

WORKDIR /squadov
RUN yarn install && yarn webpack --config ./webpack/${DEPLOYMENT_ENVIRONMENT}.config.js
RUN rm -r node_modules && yarn install --production

FROM debian:buster-20200908-slim
ARG DEPLOYMENT_ENVIRONMENT
RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*
RUN curl -sL https://deb.nodesource.com/setup_12.x | bash -
RUN apt-get update && apt-get install -y --no-install-recommends nodejs && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /squadov
RUN mkdir -p /squadov/config
COPY assets /squadov/assets
COPY config/${DEPLOYMENT_ENVIRONMENT}.json /squadov/config/config.json
COPY js /squadov/js
COPY views /squadov/views
COPY index.js /squadov
COPY package.json /squadov
COPY --from=builder /squadov/dist /squadov/dist
COPY --from=builder /squadov/node_modules /squadov/node_modules

EXPOSE 3000/tcp
WORKDIR /squadov
ENTRYPOINT ["node" , "index.js", "config/config.json"]