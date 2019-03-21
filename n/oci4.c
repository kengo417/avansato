#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <oci.h>

/* 配列の要素の数 */
#define NumberOfArray       10
#define MAX_BUF_SIZE        (64*1024)

static text *database = (text *) "D403";
static text *username = (text *)"sd4maneg";
static text *password = (text *)"sd4maneg";

/*定義*/
static struct
{
    char *ini;

} conf_str[] = {
    {"SQL=INSERT INTO emp VALUES (:EMPNO, :ENAME, :JOB, :SAL, :DEPTNO)"},
    {":EMPNO=1"},
    {":ENAME=2"},
    {":JOB=3"},
    {":SAL=4"},
    {":DEPTNO=5"},
    NULL
};

typedef struct
{
    int         pos;
    char        name[64];
    ub2         type;
    OCIBind     *bind;
    int         leng;
    void        *mem;
    int         size;
    int         csv_col;

} DEF_ITEM;

typedef struct
{
    char        *sql;
    int         num;
    DEF_ITEM    *def;
} CONF;

static OCIEnv       *envhp;
static OCIServer    *srvhp;
static OCIError     *errhp;
static OCISvcCtx    *svchp;
static OCIStmt      *stmthp;

static void checkerr(OCIError *, sword);
static void cleanup(void);
static char *GetItem(const char *, int, int, char);

static sword status;


/****************************************************************************************/
/* MAIN */
/****************************************************************************************/
int main(int argc, char **argv)
{
    int             i;
    int             idx;
    DEF_ITEM        *it;

    OCIParam        *parmh      = NULL;      /* parameter handle */
    OCIParam        *collsthd   = NULL;      /* handle to list of columns */
    OCIParam        *colhd      = NULL;      /* column handle */
    OCIDescribe     *dschp      = NULL;      /* describe handle */
    OCISession      *authp      = NULL;
    text            *namep      = NULL;
    ub1             char_semantics;
    ub2             numcols;
    ub2             col_width;
    ub2             coltyp;
    ub4             sizep;
    text            objptr[] = "EMP";
    ub4             objp_len = strlen((char *)objptr);
    char            key[32];
    char            value[MAX_BUF_SIZE];
    CONF            conf;

    /* 定義 */
    for(idx = 0; conf_str[idx].ini; idx++)
    {
        /*printf("conf_str[%d] = %s\n", i, conf_str[idx].ini);*/

        strcpy(key,   GetItem(conf_str[idx].ini, 1, sizeof(key), '='));
        strcpy(value, GetItem(conf_str[idx].ini, 2, sizeof(value), '='));

        if(strncmp(key, "SQL", strlen("SQL")) == 0)
        {
            conf.sql = malloc(strlen(value) + 1);
            strcpy(conf.sql, value);
        }
        else
        if(*(key + 0) == ':')
        {
            conf.def = realloc(conf.def, sizeof(DEF_ITEM) * (conf.num + 1));
            (conf.def + conf.num)->pos = (conf.num + 1);
            strcpy((conf.def + conf.num)->name, (key + 1));
            (conf.def + conf.num)->csv_col = atoi(value);
            conf.num++;
        }
        else
        { 
            printf("INVALID[%s:%s]\n", key, value);
        }
    }


    /***********************************************************************/
    OCIInitialize(OCI_DEFAULT, NULL, NULL, NULL, NULL);
    OCIEnvInit(&envhp, OCI_DEFAULT, 0, NULL);

    OCIHandleAlloc(envhp, (dvoid **)&errhp, OCI_HTYPE_ERROR, 0, NULL);

    /* server contexts */
    OCIHandleAlloc(envhp, (dvoid **)&srvhp, OCI_HTYPE_SERVER, 0, NULL);
    OCIHandleAlloc(envhp, (dvoid **)&svchp, OCI_HTYPE_SVCCTX, 0, NULL);

    /* NET8エイリアスはここで指定する */
    OCIServerAttach(srvhp, errhp, database, strlen(database), 0);

    /* set attribute server context in the service context */
    OCIAttrSet(svchp, OCI_HTYPE_SVCCTX, srvhp, 0, OCI_ATTR_SERVER, errhp);

    OCIHandleAlloc(envhp, (dvoid **)&authp, OCI_HTYPE_SESSION, 0, NULL);

    OCIAttrSet(authp, OCI_HTYPE_SESSION, username, strlen(username), OCI_ATTR_USERNAME, errhp);
    OCIAttrSet(authp, OCI_HTYPE_SESSION, password, strlen(password), OCI_ATTR_PASSWORD, errhp);

    checkerr(errhp, OCISessionBegin (svchp,  errhp, authp, OCI_CRED_RDBMS, OCI_DEFAULT));

    OCIAttrSet(svchp, OCI_HTYPE_SVCCTX, authp, 0, OCI_ATTR_SESSION, errhp);

    checkerr(errhp, OCIHandleAlloc(envhp, (dvoid **)&stmthp, OCI_HTYPE_STMT, 0, NULL));
    checkerr(errhp, OCIStmtPrepare(stmthp, errhp, conf.sql, strlen(conf.sql), OCI_NTV_SYNTAX, OCI_DEFAULT));

    OCIHandleAlloc(envhp, (dvoid **)&dschp, OCI_HTYPE_DESCRIBE, 0, NULL);
    /***********************************************************************/


    /***********************************************************************/
    /*カラム情報取得*/
    /***********************************************************************/
    if(OCIDescribeAny(svchp, errhp, objptr, objp_len, OCI_OTYPE_NAME, 0, OCI_PTYPE_TABLE, dschp))
    {
        printf("Error in OCIDescribeAny status[%d]\n", status);
        checkerr(errhp, status);
        cleanup();
        return OCI_ERROR;
    }

    if(OCIAttrGet(dschp, OCI_HTYPE_DESCRIBE, &parmh, NULL, OCI_ATTR_PARAM, errhp))
    {
        /*printf("NG 0\n");*/
        return OCI_ERROR;
    }

    numcols = 0;
    if(OCIAttrGet(parmh, OCI_DTYPE_PARAM, &numcols, NULL, OCI_ATTR_NUM_COLS, errhp))
    {
        /*printf("NG 1\n");*/
        return OCI_ERROR;
    }

    /* get the handle to the column list of the table */
    if(OCIAttrGet(parmh, OCI_DTYPE_PARAM, &collsthd, NULL, OCI_ATTR_LIST_COLUMNS, errhp) == OCI_NO_DATA)
    {
        /*printf("NG 2\n");*/
        return OCI_ERROR;
    }

    /* go through the column list and retrieve the data-type of each column,
    and then recursively describe column types. */
    printf("numcols = %d\n", numcols);
    for(i = 1; i <= numcols; i++)
    {
        /* get parameter for column i */
        if(OCIParamGet(collsthd, OCI_DTYPE_PARAM, errhp, (dvoid **)&colhd, i))
        {
            /*printf("NG 3\n");*/
            return OCI_ERROR;
        }

        /* for example, get datatype for ith column */
        coltyp = 0;
        if(OCIAttrGet(colhd, OCI_DTYPE_PARAM, &coltyp, NULL, OCI_ATTR_DATA_TYPE, errhp))
        {
            /*printf("NG 4\n");*/
            return OCI_ERROR;
        }

        /*カラム名
        https://docs.oracle.com/cd/A58617_01/server.804/a58234/app_exam.htm
        http://otndnld.oracle.co.jp/document/products/oracle10g/102/doc_cd/appdev.102/B19246-02/oci06des.htm */
        OCIAttrGet(colhd, OCI_DTYPE_PARAM, &namep, &sizep, OCI_ATTR_NAME, errhp);
        printf("name=%s\n", (char*)namep);

        /* Retrieve the length semantics for the column */
        char_semantics = 0;
        OCIAttrGet(parmh, OCI_DTYPE_PARAM, &char_semantics,NULL, OCI_ATTR_CHAR_USED, errhp);

        col_width = 0;
        if(char_semantics)
        {
            /* Retrieve the column width in characters */
            OCIAttrGet(colhd, OCI_DTYPE_PARAM, &col_width, NULL, OCI_ATTR_CHAR_SIZE, errhp);
        }
        else
        {
            /* Retrieve the column width in bytes
            NUMBERについては22を戻します */
            OCIAttrGet(colhd, OCI_DTYPE_PARAM, &col_width,NULL, OCI_ATTR_DATA_SIZE, errhp);
        }
        printf("size=%d  coltyp=%d\n", col_width, coltyp);

        for(idx = 0; idx < conf.num; idx++)
        {
            /*printf("comp [%s] - [%s]\n", (conf.def + idx)->name, (char*)namep);*/
            if(strcmp((conf.def + idx)->name, (char*)namep) == 0)
            {
                printf("%d : %s match! pos[%d]\n", idx, (conf.def + idx)->name, (conf.def + idx)->pos);
                switch(coltyp)
                {
                case 1: (conf.def + idx)->type = SQLT_STR; break;
                case 2: (conf.def + idx)->type = SQLT_INT; break;
                }
                (conf.def + idx)->leng = col_width;
                (conf.def + idx)->size = ((conf.def + idx)->type == SQLT_INT) ? sizeof(sword) : col_width + 1;
                (conf.def + idx)->mem = calloc((conf.def + idx)->size, NumberOfArray);

                break;
            }
        }
    }

    /***********************************************************************/
    /*BIND*/
    /***********************************************************************/
    idx = 0;
    for(idx = 0; idx < conf.num; idx++)
    {
        printf("OCIBindByPos(%d) = %d, %s, %d, %d\n", idx, (conf.def + idx)->pos, (conf.def + idx)->name, (conf.def + idx)->size, (conf.def + idx)->type);
        if(status = OCIBindByName(stmthp, &(conf.def + idx)->bind, errhp,
            (conf.def + idx)->name, -1, (conf.def + idx)->mem, (conf.def + idx)->size,
            (conf.def + idx)->type, NULL, NULL, NULL, 0, NULL, OCI_DEFAULT))
        {
            checkerr(errhp, status);
            cleanup();
            return OCI_ERROR;
        }
    }

    /*データ設定*/
    for(i = 0; i < NumberOfArray; i++)
    {
        for(idx = 0; idx < conf.num; idx++)
        {
            it = (conf.def + idx);
            switch(it->pos)
            {
                case 1:     *((sword*)(it->mem + (it->size * i))) = (sword)(i + 1);
                            break;
                case 2:     sprintf(it->mem + (it->size * i), "name_%d", i+1);
                            break;
                case 3:     sprintf(it->mem + (it->size * i), "IT%02d", i);
                            break;
                case 4:     *((sword*)(it->mem + (it->size * i))) = (sword)((i%9 + 1)* 10);
                            break;
                case 5:     *((sword*)(it->mem + (it->size * i))) = (sword)((i%4 + 1)* 10);
                            break;
            }
        }
    }

    /* 実行時に配列の要素数を指定する。*/
    if((status = OCIStmtExecute(svchp, stmthp, errhp, NumberOfArray, 0, NULL, NULL, OCI_DEFAULT)))
    {
        printf("Error in Execute\n");
        checkerr(errhp, status);
        cleanup();
        return OCI_ERROR;
    }

    if(status = OCITransCommit(svchp, errhp, OCI_DEFAULT))
    {
        checkerr(errhp, status);
        cleanup();
        return OCI_ERROR;
    }

    printf("%d inserted.\n", NumberOfArray);
    cleanup();
    return OCI_SUCCESS;
}


/****************************************************************************************/
/* checkerr */
/****************************************************************************************/
void checkerr(OCIError *errhp, sword status)
{
    text errbuf[512];
    sb4 errcode = 0;

    switch (status)
    {
        case OCI_SUCCESS:
                break;
        case OCI_SUCCESS_WITH_INFO:
                printf("Error - OCI_SUCCESS_WITH_INFO\n");
                break;
        case OCI_NEED_DATA:
                printf("Error - OCI_NEED_DATA\n");
                break;
        case OCI_NO_DATA:
                printf("Error - OCI_NODATA\n");
                break;
        case OCI_ERROR:
                OCIErrorGet(errhp, 1, NULL, &errcode, errbuf, sizeof(errbuf), OCI_HTYPE_ERROR);
                printf("Error - %.*s\n", 512, errbuf);
                break;
        case OCI_INVALID_HANDLE:
                printf("Error - OCI_INVALID_HANDLE\n");
                break;
        case OCI_STILL_EXECUTING:
                printf("Error - OCI_STILL_EXECUTE\n");
                break;
        case OCI_CONTINUE:
                printf("Error - OCI_CONTINUE\n");
                break;
        default:
                break;
    }
}


/*****************************************************************************************
 *  Exit program with an exit code.
 *****************************************************************************************/
void cleanup(void)
{
    if(stmthp)
    {
        checkerr(errhp, OCIHandleFree(stmthp, OCI_HTYPE_STMT));
    }

    if(errhp)
    {
        OCIServerDetach(srvhp, errhp, OCI_DEFAULT);
    }

    if(srvhp)
    {
        checkerr(errhp, OCIHandleFree(srvhp, OCI_HTYPE_SERVER));
    }

    if(svchp)
    {
        OCIHandleFree(svchp, OCI_HTYPE_SVCCTX);
    }

    if(errhp)
    {
        OCIHandleFree(errhp, OCI_HTYPE_ERROR);
    }

    return;
}

/*****************************************************************************************
 * GetItem
 *****************************************************************************************/
char *GetItem(const char *buf, int itemno, int size, char separater)
{
    static char work[MAX_BUF_SIZE];
    static char temp[MAX_BUF_SIZE];
    int         i;
    int         pos;
    int         nitem;
    size_t      len;
    char        bsep = 0;

    memset(work, 0, sizeof(work));
    memset(temp, 0, sizeof(temp));

    for(i = 0, pos = 0, nitem = 1; *(buf + i); i++)
    {
        if(*(buf + i) == separater && bsep == 0)
        {
            nitem++;
            bsep = 1;
            continue;
        }

        if(nitem > itemno)
        {
            break;
        }
        else
        if(nitem == itemno)
        {
            temp[pos++] = *(buf + i);
        }
    }

    len = strlen(temp);
    for(i = 0; i < (int)len; i++)
    {
        if(*(temp + i) != 0x20)
        {
            memcpy(work, (temp + i), len - i);
            break;
        }
    }

    len = strlen(work);
    for(i = len - 1; i >= 0; i--)
    {
        if(*(work + i) == 0x20)
        {
            *(work + i) = 0x00;
        }
        else
        {
            break;
        }
    }

    work[size] = 0x00;
    return work;
}

