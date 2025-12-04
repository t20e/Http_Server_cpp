# Docker for the frontend application

# Light version of Node
FROM node:18-alpine 

WORKDIR /app

# Copy pack.json first to cache dependencies
COPY package.json package-lock.json ./

# Install dependencies inside the container
RUN npm install

# Copy rest of the code into the container
COPY . .

# EXPOSE the React frontend port
EXPOSE 3000

# Start the React server
CMD ["npm", "start"]