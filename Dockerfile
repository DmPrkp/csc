FROM alpine as build-env
# install build-base meta package inside build-env container
RUN apk add --no-cache build-base
# change directory to /app
WORKDIR /app
# copy all files from current directory inside the build-env container
COPY . .
# Compile the source code and generate hello binary executable file
RUN gcc -o main main.c
# use another container to run the program
FROM alpine
# copy binary executable to new container
COPY --from=build-env /app/compile /app/compile
WORKDIR /app
# at last run the program
CMD ["/app/compile"] 