int hammingDistance(char *a, char *b, int n, int max)
{
  int mismatch = 0;
  int i;
  
  for(i = 0; i < n; i++)
  {
    if((a[i] - b[i]) != 0)
      mismatch++; 
      
    /* TODO test performance */  
    if(mismatch > max) 
      return mismatch;
  }
  
  return mismatch;
}

int editDistance(char* a, int na, char* b, int nb, int dist) {
	int oo = 0x7FFFFFFF;

	static int T[2][100];

	int ia, ib;

	int cur = 0, min;
	ia = 0;

	for (ib = 0; ib <= nb; ib++)
		T[cur][ib] = ib;

	cur = 1 - cur;

	for (ia = 1; ia <= na; ia++) {
		int ib_st = 0;
		int ib_en = nb;

		ib = 0;
		T[cur][ib] = ia;
		ib_st++;

		min = 1 << 30;

		for (ib = ib_st; ib <= ib_en; ib++) {
			int ret = oo;

			int d1 = T[1 - cur][ib] + 1;
			int d2 = T[cur][ib - 1] + 1;
			int d3 = T[1 - cur][ib - 1];
			if (a[ia - 1] != b[ib - 1])
				d3++;

			if (d1 < ret)
				ret = d1;
			if (d2 < ret)
				ret = d2;
			if (d3 < ret)
				ret = d3;

			T[cur][ib] = ret;

			/* XXX not tested */
			int difa = na - ia, difb = nb - ib, totalMin = ret + abs(
					difa - difb);

			if (totalMin < min)
				min = totalMin;
		}

		if (min > dist)
			return min;

		cur = 1 - cur;
	}

	int ret = T[1 - cur][nb];

	return ret;
}

void matchWord(char *w, int l)
{
}
