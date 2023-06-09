SRV_EXEC=srv
CLT_EXEC=clt


.PHONY: all
all: $(SRV_EXEC) $(CLT_EXEC)
	@echo "Done"

$(SRV_EXEC): server.c
	gcc server.c -o $(SRV_EXEC)

$(CLT_EXEC): client.c
	gcc client.c -o $(CLT_EXEC)

.PHONY: clean
clean:
	rm -f $(SRV_EXEC) $(CLT_EXEC)
