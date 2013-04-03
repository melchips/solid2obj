NAME		=	solid2obj

DIRECTORY	=	solid2obj

ARCHIVE		=	tar -cvzf

CC		=	gcc

ECHO		=	@echo

RM		=	rm -f

#SOURCEDIR		=	src

#INCLUDEDIR		=	include

SRC		=	solid2obj.c

CFLAGS		=	-Wall			\
			-W			\
			-Wstrict-prototypes	\
			-g                      \
			-O4                     \
			-Wformat-security       \
#			-Werror

IFLAGS		=	-I $(INCLUDEDIR)	\
			-I/usr/include		


LFLAGS		=	-L/usr/lib		\
#			-lm

OBJ		=	$(SRC:.c=.o)

all :		$(NAME)

$(NAME) :	$(OBJ)
		$(CC) $(OBJ) $(LFLAGS) -o $(NAME)

%.o: %.c
		$(CC) $(CFLAGS) $(IFLAGS) $< -c -o $@

.PHONY: clean distclean doc

clean :
		$(RM) $(OBJ)
		$(RM) *~ \#*\#

distclean :	clean
		$(RM) $(NAME)

doc :
		doxygen Doxyfile

separator :
		$(ECHO) "------------------------------------------"

re :		clean separator all

run :		all
		./$(NAME)

tarball :	distclean separator
		$(ECHO) "Archiving..."
		cd .. && $(ARCHIVE) $(NAME).tar.gz $(DIRECTORY)
		$(ECHO) "Done !"
