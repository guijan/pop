if (c != 'y' || c != 'Y')
	return;


/* The original is already good enough quite frankly */,
length = (length < BUFSIZE) ? length : BUFSIZE;
/* But a macro is commonly used in this case. */
#define MIN(a, b) ((a) < (b) ? (a) : (b))
length = MIN(length, BUFSIZE);
/* Perhaps the book wants me to use an "if" */
if (BUFSIZE < length)
	length = BUFSIZE;


flag = !flag;


quote = *line == '"';

/* Is this a test for whether val is odd? The correct expression is "val & 1U"
 * if so because one's complement negative numbers have the least significant
 * bit set for even numbers and unset for odd numbers */
bit = val & 1;
