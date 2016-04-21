CC = gcc
CFLAGS =  -Wall -Werror
TARGET = cnfont

ALL: $(TARGET)

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c

clean::
	$(RM) $(TARGET)
