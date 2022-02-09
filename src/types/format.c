/**
 * @file format.c
 *
 * File containing the implementation of the #Format type.
 *
 * The #Format type is used to represent the type of a C struct, that is, what members the
 * struct has and in what order in memory they are. This type allows to generalize many functions
 * which read / write structures from / to files, as well as performing validation .etc
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "io/taskManager.h"
#include "types/date.h"
#include "types/format.h"
#include "utils/utils.h"

/**
 * @brief Internal storage of the pair. Can be requested with getListPair()
 */
struct internal_pair
{
	int list_member;                ///< The index of the list member
    ptrdiff_t length_displacement;  ///< Displacement to address where the size of the list will be saved upon reading
	int length_member;              ///< If it exists, the index of the length member. -1 otherwise. Assumed to exist in binary types
};

/**
 * @brief Type representing the type of a C struct, i.e., the type and location of its members
 */
struct format {
    int size;					///< The size of the struct, in bytes
    int members;				///< The number of members of the struct
    FormatType *types;			///< A list of the types of the members of the struct
    ptrdiff_t *displacements;	///< A list of the displacements of the members of the struct

    INTERNAL_PAIR* lists;		/**< A list of pairs, representing the list-size relation for lists. For the lists it is set, it will store
									 the size of the list read on the specified memory address. If this address happens to be a member,
									 validation requires it to match with the member read */
    int lists_no;				///< The length of lists

	char separator;				///< The character to print between each member
};


/**
 * @brief Defines a match between a list member and the address to its length in the object.
 */
struct pair{
    int list_member;        ///< The index of the list member
    void* length_address;   ///< Pointer to sample's address where the size of the list will be saved upon reading
};


/**
 * @brief 		Get the SizeOf a struct pair object
 *
 * @return 		The sizeof a struct pair object
 */
int getSizeOfPair(){
	return sizeof(struct pair);
}
/**
 * @brief 					Adds the information to a pair
 *
 * @param pair 				The pair list to add information to
 * @param pos				The position in the list to insert the information
 * @param member 			The value of list_member parameter
 * @param length_address 	The value of length_address parameter
 */
void addToPair(struct pair* pair,int pos,int member,void*length_address){
	pair[pos].list_member = member;
	pair[pos].length_address = length_address;
}




/**
 * @brief 		Checks whether or not a ::FormatType is allocated in the heap and, therefore, needs to be freed
 *
 * @param type	The given ::FormatType
 *
 * @return 		Whether or not the ::FormatType is allocated in the heap
 */
bool isAllocd(FormatType type) {
	return (char)type >= 16;
}

/**
 * @brief 		The number of characters (bytes) a ::FormatType occupies as a string representation
 *
 * @remark 		If the exact number is not known, returns 0
 * 				(ex: INT returns 0 because it can take up 1 byte (ex: 3) or many (ex: 12345).
 * 		        BINARY_INT returns 4 because it always takes up exaclty 4 bytes)
 *
 * @param type	The given ::FormatType
 *
 * @return 0	If the exact number of bytes is not known
 * @return 		The string size, otherwise
 */
int stringSize(FormatType type) {
	switch (type) {
		case BINARY_BOOL:
			return 1;
		case BINARY_TYPE:
			return 1;
		case BINARY_INT:
			return sizeof(int);
		case BINARY_DATE_TIME:
			return BINARY_DATE_TIME_LENGTH;
		case DATE:
			return DATE_LENGTH;
		case DATE_TIME:
			return DATE_TIME_LENGTH;
		case BINARY_DOUBLE:
			return sizeof(double);
		default:
			return 0;
	}
}

/**
 * @brief 		Returns the size in bytes of the elements of the list ::FormatTypes
 *
 * @param type 	The list format type
 *
 * @return 		The string size
 */
int elemStringSize(FormatType type) {
	switch (type) {
		case BINARY_INTLIST:
			return sizeof(int);

		case STRING:
		case STRING_NULL:
			return sizeof(char);

		default:
		return 0;
	}
}

/**
 * @brief 			Reads the binary data from a string to the destination pointed to by dest
 *
 * @param type 		The ::FormatType of the member to read
 * @param str 		The member, as a string
 * @param str_len 	The length of the string
 * @param dest		The destination to write to
 */
void readBinaryMember(FormatType type, char* str, int str_len, void* dest) {
	switch (type) {
		case BINARY_BOOL:
			*(bool*)dest = (int)str[0];
			break;
		case BINARY_TYPE:
			*(Type*)dest = (int)str[0];
			break;
		case BINARY_INT:
			*(int*)dest = readIntFromBinaryString(str);
			break;
		case BINARY_DOUBLE:
			*(double*)dest = *(double*)str;
			break;
		case STRING_NULL:
		case STRING:
			if (str_len == 0)
				*(char**)dest = NULL;
			else {
				*(char**)dest = malloc(str_len + 1);
				strncpy(*(char**)dest, str, str_len);
				(*(char**)dest)[str_len] = '\0';
			}
			break;
		case BINARY_INTLIST:
			*(int**)dest = BinaryStringTointList(str,str_len/sizeof(int));
			break;
		case BINARY_DATE_TIME:
			*(Date*)dest = getUncompactedDate(readIntFromBinaryString(str));
			break;
		default:
			fprintf(stderr,"readBinaryMember: binary data type %d not recognized\n", type);
			exit(EXIT_FAILURE);
	}
}

/**
 * @brief 				Writes the member pointed to by src to the string
 *
 * @param type 			The ::FormatType of the member to write
 * @param src 			The member
 * @param dest 			The destination buffer
 * @param dest_len 		The length of the string representation of the member
 */
void writeBinaryMember(FormatType type, void* src, char* dest, int dest_len) {
	switch (type) {
		case BINARY_BOOL:
			dest[0] = (char)*(bool*)src;
			break;
		case BINARY_TYPE:
			dest[0] = (char)*(Type*)src;
			break;
		case BINARY_INT:
			writeIntToBinaryString(dest, *(int*)dest);
			break;
		case STRING_NULL:
		case STRING:
			if (*(char**)src != NULL)
				memcpy(dest, *(char**)src, dest_len);
			break;
		case BINARY_INTLIST:
			IntListToBinaryString(&dest, dest_len / sizeof(int), *(int**)src);
			break;
		case BINARY_DATE_TIME:
			writeIntToBinaryString(dest, getCompactedDate(*(Date*)src));
			break;
		default:
			fprintf(stderr,"writeBinaryMember: binary data type %d not recognized\n", type);
			exit(EXIT_FAILURE);
	}
}


/**
 * @brief 			Checks if a format could be binary, i.e. the size of each member is determined upon reading
 *
 * @param format 	The #Format to test
 *
 * @return  		Whether or not the #Format follows the binary specs
 */
bool checkBinarySpecs(Format format)
{
	int list_index = 0;

	for (int i = 0; i < format->members; i++) {
		if (stringSize(format->types[i]) == 0 &&
			(format->lists[list_index].list_member != i || elemStringSize(format->types[i]) == 0
			 || format->lists[list_index].length_member == -1 || format->lists[list_index++].length_member > i))
				return false;
	}

	return true;
}

/**
 * @brief 				Creates a #Format
 *
 * @param sample  		A sample struct of the type to create the #Format for
 * @param params  		A list with references to the members of the #Format type
 * @param types   		A list with the ::FormatType of the references passed in params
 * @param params_length The length of the lists @ref format.params and @ref format.types
 * @param size			The size of struct in bytes (result of sizeof(Type))
 * @param lists  		A list containing a pair making the correspondence between every list type (INTLIST) and their
 * 						length in the object, ordered by list_member
 * @param lists_no		Size of lists
 * @param separator		The character that separates each member in the string representation of the format. If '\0', no
 * 						character is printed and the type is assumed to be binary (i.e. every member has a constant size
 * 						(stringSize() != 0) or its size can be read from its 'lists'-paired member. Note that this means
 * 						the length element of the pair must be BEFORE the list element)
 *
 * @return	     		The requested format
 *
 */
Format makeFormat(void* sample, void** params, FormatType *types, int params_length, int size, PAIR* lists, int lists_no, char separator) {
	Format f = (Format)malloc(sizeof(struct format));

	f->size = size;
	f->members = params_length;
	f->types = (FormatType*)malloc(f->members * sizeof(FormatType));
	f->displacements = (ptrdiff_t*)malloc(f->members * sizeof(ptrdiff_t));
	f->lists = (INTERNAL_PAIR*)malloc(lists_no * sizeof(INTERNAL_PAIR));
	f->lists_no = lists_no;
	f->separator = separator;

	for (int i = 0; i < f->members; i++) {
		f->types[i] = types[i];
		f->displacements[i] = params[i] - sample;
	}

	for (int i = 0; i < f->lists_no; i++) {
		if (i > 0 && lists[i].list_member <= lists[i-1].list_member) {
			fprintf(stderr, "makeFormat: lists[%d] is invalid (not ordered by list_member)\n", i);
			disposeFormat(f);
			return NULL;
		}

		if (lists[i].list_member < 0 || lists[i].list_member >= f->members) {
			fprintf(stderr, "makeFormat: lists[%d] is invalid\n", i);
			disposeFormat(f);
			return NULL;
		}

		f->lists[i].list_member = lists[i].list_member;
		f->lists[i].length_displacement = lists[i].length_address - sample;
		f->lists[i].length_member = -1;

		for (int j = 0; j < f->members && f->lists[i].length_member == -1; j++)
			if (lists[i].length_address == params[j])
				f->lists[i].length_member = j;
	}

	if (isBinary(f) && !checkBinarySpecs(f)) {
		fprintf(stderr, "makeFormat: format has no separator but doesn't follow binary specs\n");
		disposeFormat(f);
		return NULL;
	}

	return f;
}

/**
 * @brief Copies a format
 *
 * @param f The format to copy
 * @return 	The copy of the format. Must be freed independently of f
 */
Format copyFormat(Format f)
{
	Format ans = malloc(sizeof(struct format));
	ans->size = f->size;
	ans->members = f->members;
	ans->lists_no = f->lists_no;
	ans->separator = f->separator;

	ans->types = malloc(ans->members * sizeof(FormatType));
	ans->displacements = malloc(ans->members * sizeof(ptrdiff_t));
	ans->lists = malloc(ans->lists_no * sizeof(INTERNAL_PAIR));

	memcpy(ans->types, f->types, ans->members * sizeof(FormatType));
	memcpy(ans->displacements, f->displacements, ans->members * sizeof(ptrdiff_t));
	memcpy(ans->lists, f->lists, ans->lists_no * sizeof(INTERNAL_PAIR));

	return ans;
}

/**
 * @brief 			Checks whether or not the #Format is binary
 *
 * @param format 	The format
 *
 * @return 			Wether the format is binary
 */
bool isBinary(Format format) {
	return format->separator == '\0';
}

/**
 * @brief 			Gets the size of the type stored by a #Format
 *
 * @param format 	The given #Format
 *
 * @return 			The size of type stored by the #Format
 */
int getFormatSize(Format format) {
	return format->size;
}

/**
 * @brief 			Get the number of members of the type stored by a #Format
 *
 * @param format	The given #Format
 *
 * @return 			The number of members of the #Format
 */
int getFormatMembers(Format format) {
	return format->members;
}

/**
 * @brief 				Gets the type of the specified member of a #Format
 *
 * @warning				Does not check for boundaries of the array
 *
 * @param format		The format
 * @param member_index 	The index of the member
 *
 * @return 				The type
 */
FormatType getMemberType(Format format, int member_index) {
	return format->types[member_index];
}

/**
 * @brief 				Gets the list pair with the given index from the #Format
 *
 * @warning				Does not check for boundaries of the array
 *
 * @param format 		The given #Format
 * @param pair_index 	The index
 *
 * @return 				The list pair
 */
int getListPairListMember(Format format, int pair_index) {
	return format->lists[pair_index].list_member;
}

/**
 * @brief Gets the list pair length member with the given index from the #Format
 *
 * @warning				Does not check for boundaries of the array
 *
 * @param format 		The given #Format
 * @param pair_index 	The index
 * @return 				The list pair length_member
 */
int getListPairLengthMember(Format format, int pair_index) {
	return format->lists[pair_index].length_member;
}
/**
 * @brief 				Accesses a struct and retrieves a pointer to the specified member
 *
 * @warning				Does not check for boundaries of the array
 *
 * @param format 		The #Format of the type
 * @param obj 			The object to access
 * @param index_member 	The index of the member to retrieve
 * @returns				A pointer to the member
 */
void* getMember(Format format, void* obj, int member_index) {
	return obj + format->displacements[member_index];
}

/**
 * @brief Frees the given member of a struct
 *
 * @warning				Does not check for boundaries of the array
 *
 * @param format		The #Format of the type
 * @param obj 			The struct
 * @param member_index 	The index of the member to free
 */
void freeMember(Format format, void* obj, int member_index) {
	if (isAllocd(format->types[member_index])) {
		void* *val = getMember(format, obj, member_index);
		free(*val);
	}
}


/**
 *
 * @brief 			Aux function to @ref readFormat. Frees all resources used by the first members
 * 					of the destination struct
 *
 * @warning 		Does not validate the number of members argument
 *
 * @param format    The #Format of the struct
 * @param dest    	The object whose members are to be freed
 * @param members 	The number of members to free
 */
void partialFree(Format format, void* dest, int members) {
	for (int i = 0; i < members; i++)
		freeMember(format, dest, i);
}

/**
 *
 * @brief 				Checks whether the input string can be casted into the desired #Format
 *
 * @note				If you intend to parse immediately after, use @ref readFormat
 *
 * @param f    			The #Format
 * @param str   		The text representation of the object (null terminated string). Is altered (strstep)
 * @param separators	A string containing all the characters to be used as separators. If NULL or empty,
 * 						the format is assumed to be binary (ie the size of each member is determined, stringSize() != 0)
 *
 * @return 				Whether or not the input can be parsed to #Format
 */
bool checkFormat(Format f, char* str) {

	bool is_last = false, valid = true, binary = isBinary(f);
	int temp_length, aux_list_sizes[f->members], list_index = 0;
	char *temp = NULL, *temp_ptrs[f->members], trash[sizeof(void*)], separators[2] = "";
	separators[0] = f->separator;

	for (int i = 0; valid && i < f->members; i++) {
		is_last = i == f->members - 1;
		if (binary) {
			temp = str;
			temp_length = stringSize(f->types[i]);
			//if the length is unknown it must be a binary list type
			if (temp_length == 0)
				temp_length = atoi(temp_ptrs[f->lists[list_index++].length_member]) * elemStringSize(f->types[i]);
			str += temp_length;
			if (*str == '\0') str = NULL;
		} else {
			temp = strsep(&str, separators);
			temp_length = is_last ? strlen(temp) : str - temp - 1;
		}

		if (is_last != !str) { 	//untimely end of line
			valid = false;
			break;
		}

		temp_ptrs[i] = temp;
		switch (f->types[i]) {
			case INT:
			if (!checkInt(temp, temp_length))
				valid = false;
			break;

			case STRING:
			if (temp_length == 0)
				valid = false;
			break;

			case STRING_NULL:
			break;

			case INTLIST:
			if (!checkIdList(temp, temp_length, &aux_list_sizes[i]))
				valid = false;
			break;

			case TYPE:
			if (!checkType(temp, temp_length, (Type*)trash))
				valid = false;
			break;

			case DATE:
			if (!checkDate(temp, temp_length))
				valid = false;
			break;

			case BOOL:
			if (!checkBool(temp, temp_length, (bool*)trash))
				valid = false;
			break;

			default:
			fprintf(stderr, "checkFormat: Invalid format type %d\n", f->types[i]);
			valid = false;
			break;
		}
	}

	//TODO: fix atoi (it is called without a following null terminator)
	for (int i = 0; !binary && valid && i < f->lists_no; i++)
		if (f->lists[i].length_member != -1 && aux_list_sizes[f->lists[i].list_member] != atoi(temp_ptrs[f->lists[i].length_member]))
			valid = false;

	return valid;
}

/**
 *
 * @brief 				Parses a string representation of a struct through the specified #Format
 *
 * @param f    			The #Format
 * @param str   		The string representation of the object (null terminated string). Is altered (strstep)
 * @param separators	A string containing all the characters to be used as separators. If NULL or empty,
 * 						the format is assumed to be binary (ie the size of each member is determined, stringSize() != 0)
 * @param dest			An address to the destination object. Must be of the correct type for the format
 *
 * @return  			Whether or not the conversion was successful
 */
bool readFormat(Format f, char* str, void* dest) {

	bool is_last = false, binary = isBinary(f);
	int aux_list_sizes[f->members], temp_length, list_index = 0;
	char *temp, separators[2] = "";
	separators[0] = f->separator;

	for (int i = 0; i < f->members; i++) {
		is_last = i == f->members - 1;
		if (binary) {
			temp = str;
			temp_length = stringSize(f->types[i]);
			//if the length is unknown it must be a binary list type
			if (temp_length == 0)
				temp_length = *(int*)getMember(f, dest, f->lists[list_index++].length_member) * elemStringSize(f->types[i]);
			str += temp_length;

		} else {
			temp = strsep(&str, separators);
			temp_length = is_last ? strlen(temp) : str - temp - 1;
		}

		if (!binary && is_last != !str) { //untimely end of line
			partialFree(f, dest, i);
			return false;
		}

		switch (f->types[i]) {
			case INT:
			if (!safeStringToInt(temp, temp_length, (int*)getMember(f, dest, i))) {
				partialFree(f, dest, i);
				return false;
			}
			break;

			case STRING:
			if (temp_length == 0) {
				partialFree(f, dest, i);
				return false;
			}
			*(char**)getMember(f, dest, i)=malloc(temp_length + 1);
			strncpy(*(char**)getMember(f, dest, i),temp,temp_length);
			(*(char**)getMember(f, dest, i))[temp_length] = '\0';
			aux_list_sizes[i] = temp_length;
			break;

			case STRING_NULL:
			if (temp_length == 0)
				*(char**)getMember(f, dest, i)=NULL;
			else {
				*(char**)getMember(f, dest, i)=malloc(temp_length + 1);
				strncpy(*(char**)getMember(f, dest, i), temp, temp_length);
				(*(char**)getMember(f, dest, i))[temp_length] = '\0';
			}
			aux_list_sizes[i] = temp_length;
			break;

			case INTLIST:
			if (!readIdList(temp, temp_length, (int**)getMember(f, dest, i), &aux_list_sizes[i])) {
				partialFree(f, dest, i);
				return false;
			}
			break;

			case TYPE:
			if (!checkType(temp, temp_length, (Type*)getMember(f, dest, i))) {
				partialFree(f, dest, i);
				return false;
			}
			break;

			case DATE:
			case DATE_TIME:
			if (!readDate(temp, temp_length, (Date *)getMember(f, dest, i),f->types[i] == DATE_TIME)) {
				partialFree(f, dest, i);
				return false;
			}
			break;

			case BOOL:
			if (!checkBool(temp, temp_length, (bool*)getMember(f, dest, i))) {
				partialFree(f, dest, i);
				return false;
			}
			break;
			default:
			fprintf(stderr, "readFormat: Invalid format type %d\n", f->types[i]);
			partialFree(f, dest, i);
			return false;
		}
	}

	for (int i = 0; i < f->lists_no; i++) {
		if (f->lists[i].length_member == -1)
			*(int*)(dest + f->lists[i].length_displacement) = aux_list_sizes[f->lists[i].list_member];
		else if (aux_list_sizes[f->lists[i].list_member] != *(int*)getMember(f, dest, f->lists[i].length_member)){
			partialFree(f, dest, f->members);
			return false;
		}
	}

	return true;
}

/**
 *
 * @brief 				Parses a string representation of a type through the specified #Format
 *
 * @warning				This function does not check or validate the input
 *
 * @param f     		The #Format
 * @param str   		The string representation of the object (null terminated string). Is altered (strstep)
 * @param separators	A string containing all the characters to be used as separators. If NULL or empty,
 * 						the format is assumed to be binary (ie the size of each member is either determined,
 * 						stringSize() != 0, or specified beforehand)
 * @param dest  		An address to the destination object. Must be of the correct type for the format
 */
void unsafeReadFormat(Format f, char* str, void* dest) {
	int temp_length, aux_list_sizes[f->members], list_index = 0;
	bool binary = isBinary(f);
	char *temp, separators[2] = "";
	separators[0] = f->separator;

	for (int i = 0; i < f->members; i++) {
		if (binary) {
			temp = str;
			temp_length = stringSize(f->types[i]);
			//if the length is unknown it must be a binary list type
			if (temp_length == 0)
				temp_length = *(int*)getMember(f, dest, f->lists[list_index++].length_member) * elemStringSize(f->types[i]);
			str += temp_length;
			//if (*str == '\0') str = NULL;
		} else {
			temp = strsep(&str, separators);
			temp_length = (i == f->members - 1) ? strlen(temp) : str - temp - 1;
		}

		switch (f->types[i]) {
			case INT:
				*(int *)getMember(f, dest, i) = atoi(temp);
				break;
			case INTLIST:
            	*(int**)getMember(f, dest, i) = unsafeReadIdList(temp, temp_length, &aux_list_sizes[i]);
				break;
			case TYPE:
				checkType(temp, temp_length, (Type*)getMember(f, dest, i));
				break;
			case DATE_TIME:
				*(Date *)getMember(f, dest, i) = unsafeDateFromString(temp);
				break;
			case BOOL:
				checkBool(temp, temp_length, (bool*)getMember(f, dest, i));
				break;
			case BINARY_DOUBLE:
			case BINARY_TYPE:
			case BINARY_INT:
			case BINARY_INTLIST :
			case BINARY_DATE_TIME :
			case BINARY_BOOL:
			case STRING_NULL:
			case STRING:
				readBinaryMember(f->types[i],temp,temp_length,getMember(f, dest, i));
				break;
			default:
			fprintf(stderr, "Not implemented\n");
			exit(EXIT_FAILURE);
		}
	}
	if (!binary)
		for (int i = 0; i < f->lists_no; i++)
			*(int*)(dest + f->lists[i].length_displacement) = aux_list_sizes[f->lists[i].list_member];

}

/**
 *
 * @brief 				Prints a string representation of the passed type through the specified #Format
 *
 * @param f       		The #Format
 * @param src     		The struct
 * @param separators	The character to be printed between members. if '\0', nothing is printed
 * @param dest    		The output buffer
 */
void printFormat(Format f, void* src, FILE* dest) {
	int aux_list_sizes[f->members];

	for (int i = 0; i < f->lists_no; i++)
		aux_list_sizes[f->lists[i].list_member] = *(int*)(src + f->lists[i].length_displacement);

	for (int i = 0; i < f->members; i++) {
		switch (f->types[i]) {
			case INT:
			fprintf(dest, "%d", *(int*)getMember(f, src, i));
			break;

			case STRING:
			case STRING_NULL:;
			char* s = *(char**)getMember(f, src, i);
			if (s != NULL)
				fprintf(dest, "%s", s);
			break;

			case INTLIST:
			printAr(*(int**)getMember(f, src, i), dest, aux_list_sizes[i]);
			break;

			case TYPE:
			fprintf(dest, "%s", printType(*(Type*)getMember(f, src, i)));
			break;

			case DATE_TIME:
			case DATE:
			printDate(*(Date *)getMember(f, src, i), dest);
			break;

			case BOOL:
			fprintf(dest, "%s", *(bool*)getMember(f, src, i) ? "True" : "False");
			break;

			case BINARY_BOOL:;
				char cbb= (char)(*(bool*)getMember(f, src, i));
				fwrite(&cbb,1,1,dest);
				break;
			case BINARY_TYPE:;
				char cbt = (char)(*(int*)getMember(f, src, i));
				fwrite(&cbt,1,1,dest);
				break;
			case BINARY_INT:;
				char sbi[sizeof(int)];
				writeIntToBinaryString(sbi,*(int*)getMember(f, src, i));
				fwrite(sbi,sizeof(char),sizeof(int),dest);
				break;
			case BINARY_DOUBLE:;
				char* sbid=(char*)((double*)getMember(f, src, i));
				fwrite(sbid,sizeof(char),sizeof(double),dest);
				break;
			case BINARY_INTLIST:;
				char*sbil = NULL;
				IntListToBinaryString(&sbil, aux_list_sizes[i],*(int**)getMember(f, src, i));
				fwrite(sbil,sizeof(char),4*aux_list_sizes[i],dest);
				free(sbil);
				break;
			case BINARY_DATE_TIME:;
				char sbdt[sizeof(int)];
				writeIntToBinaryString(sbdt,getCompactedDate(*(Date*)getMember(f, src, i)));
				fwrite(sbdt,sizeof(char),sizeof(int),dest);
				break;
			default:
			fprintf(stderr, "printFormat: Invalid format type %d\n", f->types[i]);
		}

		if (i != f->members - 1 && f->separator != '\0')
			fprintf(dest, "%c", f->separator);
	}
}

/**
 * @brief Frees all information inside the formatted object.
 *
 * @warning		Does not free the object itself
 *
 * @param format 	The #Format
 * @param obj		The formatted object
 */
void freeFormat(Format format, void* obj) {
	partialFree(format, obj, format->members);
}


/**
 * @brief Frees the memory allocated to a #Format
 *
 * @param format	The #Format to be disposed
 */
void disposeFormat(Format format) {
	free(format->types);
	free(format->displacements);
	free(format->lists);
	free(format);
}
