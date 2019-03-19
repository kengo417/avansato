/* 
簡単な配列バインドのサンプル arraybind1.c

サンプルを実行する前に、
scott/tiger@exampledb
で接続して、次のSQLでEMP1を作成してください。
CREATE TABLE EMP1
       (EMPNO NUMBER(4) NOT NULL,
        ENAME VARCHAR2(10),
        JOB VARCHAR2(9),
        SAL NUMBER(7,2),
        DEPTNO NUMBER(2));
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oci.h>

/* 配列の要素の数 */
#define NumberOfArray 10

static text *username = (text *) "sd4maneg";
static text *password = (text *) "sd4maneg";

/* Define SQL statements to be used in program. */
//static text *insert = (text *) "INSERT INTO emp1(empno, ename, job, sal, deptno) VALUES (:empno, :ename, :job, :sal, :deptno)";
static text *insert = (text *) "INSERT INTO emp VALUES (:empno, :ename, :job, :sal, :deptno)";

static OCIEnv *envhp;
static OCIServer *srvhp;
static OCIError *errhp;
static OCISvcCtx *svchp;
static OCIStmt *stmthp;
static OCIDefine *defnp = (OCIDefine *) 0;

static OCIBind  *bnd1p = (OCIBind *) 0;             /* the first bind handle */
static OCIBind  *bnd2p = (OCIBind *) 0;             /* the second bind handle */
static OCIBind  *bnd3p = (OCIBind *) 0;             /* the third bind handle */
static OCIBind  *bnd4p = (OCIBind *) 0;             /* the fourth bind handle */
static OCIBind  *bnd5p = (OCIBind *) 0;             /* the fifth bind handle */

static void checkerr(/*_ OCIError *errhp, sword status _*/);
static void cleanup(/*_ void _*/);
static void myfflush(/*_ void _*/);
int main(/*_ int argc, char *argv[] _*/);

static sword status;

int main(argc, argv)
int argc;
char *argv[];
{
  int      i;
  sword    empno[NumberOfArray], sal[NumberOfArray], deptno[NumberOfArray];
  char     ename[NumberOfArray][11];
  char     job[NumberOfArray][10];

  text objptr[] = "EMP";
  ub4 objp_len = (ub4) strlen((char *)objptr);
OCIParam *parmh = (OCIParam *) 0;         /* parameter handle */
OCIParam *collsthd = (OCIParam *) 0;      /* handle to list of columns */
OCIParam *colhd = (OCIParam *) 0;         /* column handle */
OCIDescribe *dschp = (OCIDescribe *)0;      /* describe handle */
ub2          numcols, col_width;
ub1          char_semantics;
ub2  coltyp;
	
	text  *namep;
	ub4   sizep;


  OCIDescribe  *dschndl1 = (OCIDescribe *) 0,
               *dschndl2 = (OCIDescribe *) 0,
               *dschndl3 = (OCIDescribe *) 0;
  OCISession *authp = (OCISession *) 0;
  
  (void) OCIInitialize((ub4) OCI_DEFAULT, (dvoid *)0,
                       (dvoid * (*)(dvoid *, size_t)) 0,
                       (dvoid * (*)(dvoid *, dvoid *, size_t))0,
                       (void (*)(dvoid *, dvoid *)) 0 );

  (void) OCIEnvInit( (OCIEnv **) &envhp, OCI_DEFAULT, (size_t) 0, (dvoid **) 0 );

  (void) OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &errhp, OCI_HTYPE_ERROR, 
                   (size_t) 0, (dvoid **) 0);

  /* server contexts */
  (void) OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &srvhp, OCI_HTYPE_SERVER,
                   (size_t) 0, (dvoid **) 0);

  (void) OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &svchp, OCI_HTYPE_SVCCTX,
                   (size_t) 0, (dvoid **) 0);

  /* NET8エイリアスはここで指定する */
  (void) OCIServerAttach( srvhp, errhp, (text *)"d403", strlen("d403"), 0);

  /* set attribute server context in the service context */
  (void) OCIAttrSet( (dvoid *) svchp, OCI_HTYPE_SVCCTX, (dvoid *)srvhp, (ub4) 0,
                    OCI_ATTR_SERVER, (OCIError *) errhp);

  (void) OCIHandleAlloc((dvoid *) envhp, (dvoid **)&authp, (ub4) OCI_HTYPE_SESSION,
       (size_t) 0, (dvoid **) 0);
 
  (void) OCIAttrSet((dvoid *) authp, (ub4) OCI_HTYPE_SESSION,
                 (dvoid *) username, (ub4) strlen((char *)username),
                 (ub4) OCI_ATTR_USERNAME, errhp);
 
  (void) OCIAttrSet((dvoid *) authp, (ub4) OCI_HTYPE_SESSION,
                 (dvoid *) password, (ub4) strlen((char *)password),
                 (ub4) OCI_ATTR_PASSWORD, errhp);

  checkerr(errhp, OCISessionBegin ( svchp,  errhp, authp, OCI_CRED_RDBMS, 
        (ub4) OCI_DEFAULT));

  (void) OCIAttrSet((dvoid *) svchp, (ub4) OCI_HTYPE_SVCCTX,
                   (dvoid *) authp, (ub4) 0,
                   (ub4) OCI_ATTR_SESSION, errhp);

  checkerr(errhp, OCIHandleAlloc( (dvoid *) envhp, (dvoid **) &stmthp,
           OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0));

  checkerr(errhp, OCIStmtPrepare(stmthp, errhp, insert, 
                                (ub4) strlen((char *) insert),
                                (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT));

  if (status = OCIBindByName(stmthp, &bnd1p, errhp, (text *) ":ENAME",
             -1, (dvoid *) ename[0],
             (sb4) sizeof(ename[0]), SQLT_STR, (dvoid *) 0,
             (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT))
  {
    printf("Error in Bind JOB\n");
    checkerr(errhp, status);
    cleanup();
    return OCI_ERROR;
  }
  if (status = OCIBindByName(stmthp, &bnd2p, errhp, (text *) ":JOB",
             -1, (dvoid *) job[0],
             (sb4) sizeof(job[0]), SQLT_STR, (dvoid *) 0,
             (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT))
  {
    printf("Error in Bind JOB\n");
    checkerr(errhp, status);
    cleanup();
    return OCI_ERROR;
  }
  if (status = OCIBindByName(stmthp, &bnd3p, errhp, (text *) ":SAL",
             -1, (dvoid *) &sal[0],
             (sb4) sizeof(sal[0]), SQLT_INT, (dvoid *) 0,
             (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT))
  {
    printf("Error in Bind SAL\n");
    checkerr(errhp, status);
    cleanup();
    return OCI_ERROR;
  }
  if (status = OCIBindByName(stmthp, &bnd4p, errhp, (text *) ":DEPTNO",
             -1, (dvoid *) &deptno[0],
             (sb4) sizeof(deptno[0]), SQLT_INT, (dvoid *) 0,
             (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT))
  {
    printf("Error in Bind DEPTNO\n");
    checkerr(errhp, status);
    cleanup();
    return OCI_ERROR;
  }
  if (status = OCIBindByName(stmthp, &bnd5p, errhp, (text *) ":EMPNO",
             -1, (dvoid *) &empno[0],
             (sb4) sizeof(empno[0]), SQLT_INT, (dvoid *) 0,
             (ub2 *) 0, (ub2 *) 0, (ub4) 0, (ub4 *) 0, OCI_DEFAULT))
  {
    printf("Error in Bind EMPNO\n");
    checkerr(errhp, status);
    cleanup();
    return OCI_ERROR;
  }

  for( i = 0; i < NumberOfArray; i++)
  {
    sprintf(ename[i],"name_%d",i+1 );
    strcpy(job[i],"IT");
    sal[i] = (i%9 + 1) * 1000;
    deptno[i] = (i%4 + 1) * 10;
    empno[i] = i+1;
  }

  OCIHandleAlloc((dvoid *)envhp, (dvoid **)&dschp, (ub4)OCI_HTYPE_DESCRIBE, (size_t)0, (dvoid **)0);

  if (OCIDescribeAny(svchp, errhp, (dvoid *)objptr, objp_len, OCI_OTYPE_NAME, 0, OCI_PTYPE_TABLE, dschp))
  {
    printf("Error in OCIDescribeAny status[%d]\n", status);
    checkerr(errhp, status);
    cleanup();
      return OCI_ERROR;
  }

  if (OCIAttrGet((dvoid *)dschp, OCI_HTYPE_DESCRIBE, (dvoid *)&parmh, (ub4 *)0, OCI_ATTR_PARAM, errhp))
  {
      printf("NG 0\n");
      return OCI_ERROR;
  }

  /* The type information of the object, in this case, OCI_PTYPE_TABLE,
  is obtained from the parameter descriptor returned by the OCIAttrGet(). */
  /* get the number of columns in the table */
  numcols = 0;
  if (OCIAttrGet((dvoid *)parmh, OCI_DTYPE_PARAM, (dvoid *)&numcols, (ub4 *)0, OCI_ATTR_NUM_COLS, errhp))
  {
      printf("NG 1\n");
      return OCI_ERROR;
  }

  /* get the handle to the column list of the table */
  if (OCIAttrGet((dvoid *)parmh, OCI_DTYPE_PARAM, (dvoid *)&collsthd, (ub4 *)0, OCI_ATTR_LIST_COLUMNS, errhp)==OCI_NO_DATA)
  {
      printf("NG 2\n");
      return OCI_ERROR;
  }

  /* go through the column list and retrieve the data-type of each column,
  and then recursively describe column types. */

  printf("numcols = %d\n", numcols);
  for (i = 1; i <= numcols; i++)
  {
      /* get parameter for column i */
      if (OCIParamGet((dvoid *)collsthd, OCI_DTYPE_PARAM, errhp, (dvoid **)&colhd, (ub4)i))
      {
          printf("NG 3\n");
          return OCI_ERROR;
      }

      /* for example, get datatype for ith column */
      coltyp = 0;
      if (OCIAttrGet((dvoid *)colhd, OCI_DTYPE_PARAM, (dvoid *)&coltyp, (ub4 *)0, OCI_ATTR_DATA_TYPE, errhp))
      {
          printf("NG 4\n");
          return OCI_ERROR;
      }

      /*カラム名
        https://docs.oracle.com/cd/A58617_01/server.804/a58234/app_exam.htm
        http://otndnld.oracle.co.jp/document/products/oracle10g/102/doc_cd/appdev.102/B19246-02/oci06des.htm */
      OCIAttrGet((dvoid*) colhd, (ub4) OCI_DTYPE_PARAM,
                    (dvoid*) &namep, (ub4 *) &sizep,
                    (ub4) OCI_ATTR_NAME, (OCIError *) errhp);  	
      printf("name=%s\n", (char*)namep);

      /* Retrieve the length semantics for the column */
      char_semantics = 0;
      OCIAttrGet((dvoid*) parmh, (ub4) OCI_DTYPE_PARAM,
                   (dvoid*) &char_semantics,(ub4 *) 0, (ub4) OCI_ATTR_CHAR_USED,
                   (OCIError *) errhp);

      col_width = 0;
      if (char_semantics)
      {
          /* Retrieve the column width in characters */
          OCIAttrGet((dvoid*) colhd, (ub4) OCI_DTYPE_PARAM,
                   (dvoid*) &col_width, (ub4 *) 0, (ub4) OCI_ATTR_CHAR_SIZE,
                   (OCIError *) errhp);
      }
      else
      {
          /* Retrieve the column width in bytes
             NUMBERについては22を戻します */
          OCIAttrGet((dvoid*) colhd, (ub4) OCI_DTYPE_PARAM,
                   (dvoid*) &col_width,(ub4 *) 0, (ub4) OCI_ATTR_DATA_SIZE,
                   (OCIError *) errhp);
      }
      printf("size=%d\n", col_width);
  }



  /* 実行時に配列の要素数を指定する。*/
  if ((status = OCIStmtExecute(svchp, stmthp, errhp, (ub4) NumberOfArray, (ub4) 0,
             (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DEFAULT))) 
  {
    printf("Error in Execute\n");
    checkerr(errhp, status);
    cleanup();
    return OCI_ERROR;
  }

  if (status = OCITransCommit(svchp, errhp, 0))
  {
    checkerr(errhp, status);
    cleanup();
    return OCI_ERROR;
  }

  printf("%d行INSERTしました。\n",NumberOfArray);
  cleanup();
  return OCI_SUCCESS;
}


void checkerr(errhp, status)
OCIError *errhp;
sword status;
{
  text errbuf[512];
  sb4 errcode = 0;

  switch (status)
  {
  case OCI_SUCCESS:
    break;
  case OCI_SUCCESS_WITH_INFO:
    (void) printf("Error - OCI_SUCCESS_WITH_INFO\n");
    break;
  case OCI_NEED_DATA:
    (void) printf("Error - OCI_NEED_DATA\n");
    break;
  case OCI_NO_DATA:
    (void) printf("Error - OCI_NODATA\n");
    break;
  case OCI_ERROR:
    (void) OCIErrorGet((dvoid *)errhp, (ub4) 1, (text *) NULL, &errcode,
                        errbuf, (ub4) sizeof(errbuf), OCI_HTYPE_ERROR);
    (void) printf("Error - %.*s\n", 512, errbuf);
    break;
  case OCI_INVALID_HANDLE:
    (void) printf("Error - OCI_INVALID_HANDLE\n");
    break;
  case OCI_STILL_EXECUTING:
    (void) printf("Error - OCI_STILL_EXECUTE\n");
    break;
  case OCI_CONTINUE:
    (void) printf("Error - OCI_CONTINUE\n");
    break;
  default:
    break;
  }
}


/*
 *  Exit program with an exit code.
 */
void cleanup()
{
  if (stmthp)
    checkerr(errhp, OCIHandleFree((dvoid *) stmthp, OCI_HTYPE_STMT));

  if (errhp)
    (void) OCIServerDetach( srvhp, errhp, OCI_DEFAULT );
  if (srvhp)
    checkerr(errhp, OCIHandleFree((dvoid *) srvhp, OCI_HTYPE_SERVER));
  if (svchp)
    (void) OCIHandleFree((dvoid *) svchp, OCI_HTYPE_SVCCTX);
  if (errhp)
    (void) OCIHandleFree((dvoid *) errhp, OCI_HTYPE_ERROR);
  return;
}


void myfflush()
{
  eb1 buf[50];

  fgets((char *) buf, 50, stdin);
}
