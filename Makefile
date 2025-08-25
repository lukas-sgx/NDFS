CC       = gcc
OUTPUT   = build

SRCS_SERVER = server/main.c
SRCS_CLIENT = client/main.c

SERVER   = $(OUTPUT)/server
CLIENT   = $(OUTPUT)/client

all: build_dir $(SERVER) $(CLIENT)

build_dir:
	@echo "📂 Creating build directory..."
	@mkdir -p $(OUTPUT)

$(SERVER): $(SRCS_SERVER)
	@echo "Compiling NTDFS -> SERVER 🛠️" && $(CC) $< -o $@

$(CLIENT): $(SRCS_CLIENT)
	@echo "Compiling NTDFS -> CLIENT 🛠️" && $(CC) $< -o $@

run-server: $(SERVER)
	@echo "🚀 Launch Server 👤" && ./$(SERVER)

run-client: $(CLIENT)
	@echo "🚀 Launch Client 👤" && ./$(CLIENT)

clean:
	@echo "🧹 Cleaning build files..."
	@rm -rf $(OUTPUT)
