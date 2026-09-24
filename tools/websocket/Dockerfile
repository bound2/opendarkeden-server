FROM node:22-alpine
WORKDIR /gateway
COPY package.json package-lock.json ./
RUN npm ci --omit=dev --ignore-scripts
COPY gateway.mjs ./
USER node
CMD ["node", "gateway.mjs", "/config/config.json"]
