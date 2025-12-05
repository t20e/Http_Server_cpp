# Docker for the frontend application
# Using Multi-Stage Build: Which makes the final container size go from being 500+MB to ~78MB!

# ---------------------- Stage 1: Build React App ----------------------

# Light version of Node
FROM node:18-alpine AS builder

WORKDIR /app

# Copy pack.json first to cache dependencies
COPY package.json package-lock.json ./
RUN npm ci --omit=dev

# Copy rest of the code into the container
COPY . .

# Build the react build
RUN npm run build


# ---------------------- Stage 2: Serve The Build ----------------------
FROM nginx:stable-alpine

# Remove the default nginx config
RUN rm /etc/nginx/conf.d/default.conf

# Copy the new config 
COPY nginx.conf /etc/nginx/conf.d/default.conf

# Copy the react build output to the container
COPY --from=builder /app/build /usr/share/nginx/html

# Expose the React frontend port
EXPOSE 80

# Serve the React app with nginx
CMD [ "nginx", "-g", "daemon off;" ]