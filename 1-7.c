#if 0
if (istty(stdin));
else if (istty(stdout));
	else if (istty(stderr));
		else return(0);

#endif

if (istty(stdin)) {
	;
} else if (istty(stdout)) {
	;
} else if (istty(stderr)) {
	;
} else {
	return(0);
}


#if 0
if (retval != SUCCESS) {
	return (retval);
}
/* All went well! */
return SUCCESS;
#endif

return (retval);


#if 0
for (k = 0; k++ < 5; x += dx)
	scanf("%lf", &dx);
#endif

for (k = 0; k < 5; k++, x += dx)
	scanf("%lf", &dx);

#if 0
int count = 0;
while (count < total) {
	count++;
	if (this.getName(count) == nametable.userName()) {
		return (true);
	}
}
#endif

for (int count = 0; count < total; count++)
	if (this.getName(count) == nametable.userName())
		return (true);
