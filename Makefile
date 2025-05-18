CC = gcc
CFLAGS = -Wall -Wextra -Icore -Iui -Iutils

OBJDIR = obj

SRC = main.c \
      implementation/analyzer.c \
      implementation/collector.c \
      implementation/reporter.c \
      ui/cli.c 


OBJ = $(patsubst %.c,$(OBJDIR)/%.o,$(SRC))

TARGET = minimon

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@

$(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)  # Gerekli dizinleri oluştur
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean