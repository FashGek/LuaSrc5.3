﻿/*
** $Id: lobject.h,v 2.117.1.1 2017/04/19 17:39:34 roberto Exp $
** Type definitions for Lua objects
** See Copyright Notice in lua.h
*/


#ifndef lobject_h
#define lobject_h


#include <stdarg.h>


#include "llimits.h"
#include "lua.h"


/*
** Extra tags for non-values
*/

// LUA_TPROTO代表函数原型类型，由于这并不是一个公开类型，最终用户无法得到一个Proto类型，所以LUA_TPROTO没有定义在lua.h中，而存在于lobject.h中
// （LUA_TNONE排除在外，这个类型对lua不可见提供Lua API使用）
#define LUA_TPROTO	LUA_NUMTAGS		/* function prototypes */
// LUA_TDEADKEY用来标记lua数据的上限tag
#define LUA_TDEADKEY	(LUA_NUMTAGS+1)		/* removed keys in tables */

/*
** number of all possible tags (including LUA_TNONE but excluding DEADKEY)
*/
#define LUA_TOTALTAGS	(LUA_TPROTO + 2)


/*
** tags for Tagged Values have the following use of bits:
** bits 0-3: actual tag (a LUA_T* value)
** bits 4-5: variant bits
** bit 6: whether value is collectable
*/


/*
** LUA_TFUNCTION variants:
** 0 - Lua function
** 1 - light C function
** 2 - regular C function (closure)
*/

/* Variant tags for functions */
#define LUA_TLCL	(LUA_TFUNCTION | (0 << 4))  /* Lua closure 110<=>6*/
#define LUA_TLCF	(LUA_TFUNCTION | (1 << 4))  /* light C function 10110<=>22*/
#define LUA_TCCL	(LUA_TFUNCTION | (2 << 4))  /* C closure 100110<=>38*/


/* Variant tags for strings */
#define LUA_TSHRSTR	(LUA_TSTRING | (0 << 4))  /* short strings 100<=>4*/
#define LUA_TLNGSTR	(LUA_TSTRING | (1 << 4))  /* long strings 10100<=>20*/


/* Variant tags for numbers */
#define LUA_TNUMFLT	(LUA_TNUMBER | (0 << 4))  /* float numbers 011<=>3*/
#define LUA_TNUMINT	(LUA_TNUMBER | (1 << 4))  /* integer numbers 10011<=>19*/


/* Bit mark for collectable types */
#define BIT_ISCOLLECTABLE	(1 << 6)

/* mark a tag as collectable */
#define ctb(t)			((t) | BIT_ISCOLLECTABLE)


/*
** Common type for all collectable objects
*/
typedef struct GCObject GCObject;


/*
** Common Header for all collectable objects (in macro form, to be
** included in other objects)
*/
#define CommonHeader	GCObject *next; lu_byte tt; lu_byte marked


/*
** Common type has only the common header
*/
struct GCObject {
	CommonHeader;
};




/*
** Tagged Values. This is the basic representation of values in Lua,
** an actual value plus a tag with its type.
*/

/*
** Union of all Lua values
*/
typedef union Value {
	GCObject *gc;    /* collectable objects */
	void *p;         /* light userdata */
	int b;           /* booleans */
	lua_CFunction f; /* light C functions */
	lua_Integer i;   /* integer numbers */
	lua_Number n;    /* float numbers */
} Value;


#define TValuefields	Value value_; int tt_


typedef struct lua_TValue {
	TValuefields;
} TValue;



/* macro defining a nil value */
#define NILCONSTANT	{NULL}, LUA_TNIL  //lua_TValue = {NILCONSTANT}


#define val_(o)		((o)->value_)


/* raw type tag of a TValue */
#define rttype(o)	((o)->tt_)

/* tag with no variants (bits 0-3) */
#define novariant(x)	((x) & 0x0F)	//屏蔽高位，只留低4位，

/* type tag of a TValue (bits 0-3 for tags + variant bits 4-5) */ 
#define ttype(o)	(rttype(o) & 0x3F)	//屏蔽高位，

/* type tag of a TValue with no variants (bits 0-3) */ 
#define ttnov(o)	(novariant(rttype(o)))

#pragma region ISType()宏
//Is()宏
/* Macros to test type */
#define checktag(o,t)		(rttype(o) == (t))
#define checktype(o,t)		(ttnov(o) == (t))
#define ttisnumber(o)		checktype((o), LUA_TNUMBER)
#define ttisfloat(o)		checktag((o), LUA_TNUMFLT)
#define ttisinteger(o)		checktag((o), LUA_TNUMINT)
#define ttisnil(o)		checktag((o), LUA_TNIL)
#define ttisboolean(o)		checktag((o), LUA_TBOOLEAN)
#define ttislightuserdata(o)	checktag((o), LUA_TLIGHTUSERDATA)
#define ttisstring(o)		checktype((o), LUA_TSTRING)
#define ttisshrstring(o)	checktag((o), ctb(LUA_TSHRSTR))
#define ttislngstring(o)	checktag((o), ctb(LUA_TLNGSTR))
#define ttistable(o)		checktag((o), ctb(LUA_TTABLE))
#define ttisfunction(o)		checktype(o, LUA_TFUNCTION)
#define ttisclosure(o)		((rttype(o) & 0x1F) == LUA_TFUNCTION)
#define ttisCclosure(o)		checktag((o), ctb(LUA_TCCL))
#define ttisLclosure(o)		checktag((o), ctb(LUA_TLCL))
#define ttislcf(o)		checktag((o), LUA_TLCF)
#define ttisfulluserdata(o)	checktag((o), ctb(LUA_TUSERDATA))
#define ttisthread(o)		checktag((o), ctb(LUA_TTHREAD))
#define ttisdeadkey(o)		checktag((o), LUA_TDEADKEY)
#pragma endregion

#pragma region Get()宏
//Get()宏
/* Macros to access values */
#define ivalue(o)	check_exp(ttisinteger(o), val_(o).i)
#define fltvalue(o)	check_exp(ttisfloat(o), val_(o).n)
#define nvalue(o)	check_exp(ttisnumber(o), \
	(ttisinteger(o) ? cast_num(ivalue(o)) : fltvalue(o)))
#define gcvalue(o)	check_exp(iscollectable(o), val_(o).gc)
#define pvalue(o)	check_exp(ttislightuserdata(o), val_(o).p)
#define tsvalue(o)	check_exp(ttisstring(o), gco2ts(val_(o).gc))
#define uvalue(o)	check_exp(ttisfulluserdata(o), gco2u(val_(o).gc))
#define clvalue(o)	check_exp(ttisclosure(o), gco2cl(val_(o).gc))
#define clLvalue(o)	check_exp(ttisLclosure(o), gco2lcl(val_(o).gc))
#define clCvalue(o)	check_exp(ttisCclosure(o), gco2ccl(val_(o).gc))
#define fvalue(o)	check_exp(ttislcf(o), val_(o).f)
#define hvalue(o)	check_exp(ttistable(o), gco2t(val_(o).gc))
#define bvalue(o)	check_exp(ttisboolean(o), val_(o).b)
#define thvalue(o)	check_exp(ttisthread(o), gco2th(val_(o).gc))
/* a dead value may get the 'gc' field, but cannot access its contents */
#define deadvalue(o)	check_exp(ttisdeadkey(o), cast(void *, val_(o).gc))

#define l_isfalse(o)	(ttisnil(o) || (ttisboolean(o) && bvalue(o) == 0)) //如果当前类型是nil或者当前类型是boolean但是为false/0 则表示false


#define iscollectable(o)	(rttype(o) & BIT_ISCOLLECTABLE)	//检测tag标志位的第6位，是否是gc对象


/* Macros for internal tests */
#define righttt(obj)		(ttype(obj) == gcvalue(obj)->tt)	//这里返回obj的tt_是否等于gc里面的tt  但是有什么用？？？？？？？？？？？？？？？

// 检查obj的生存期
// iscollectable(obj)检查obj是否为GC对象
// righttt(obj)返回obj的tt_是否等于gc里面的tt
// isdead(obj)返回obj是否已经被清理
// 总而言之，返回true代表未被GC的和不需要GC的，返回false代表已经被GC了的
#define checkliveness(L,obj) \
	lua_longassert(!iscollectable(obj) || \
		(righttt(obj) && (L == NULL || !isdead(G(L),gcvalue(obj)))))
#pragma endregion


#pragma region Set()宏

/* Macros to set values */
#define settt_(o,t)	((o)->tt_=(t))

#define setfltvalue(obj,x) \
  { TValue *io=(obj); val_(io).n=(x); settt_(io, LUA_TNUMFLT); }

#define chgfltvalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisfloat(io)); val_(io).n=(x); }

#define setivalue(obj,x) \
  { TValue *io=(obj); val_(io).i=(x); settt_(io, LUA_TNUMINT); }

#define chgivalue(obj,x) \
  { TValue *io=(obj); lua_assert(ttisinteger(io)); val_(io).i=(x); }

#define setnilvalue(obj) settt_(obj, LUA_TNIL)

#define setfvalue(obj,x) \
  { TValue *io=(obj); val_(io).f=(x); settt_(io, LUA_TLCF); }

#define setpvalue(obj,x) \
  { TValue *io=(obj); val_(io).p=(x); settt_(io, LUA_TLIGHTUSERDATA); }

#define setbvalue(obj,x) \
  { TValue *io=(obj); val_(io).b=(x); settt_(io, LUA_TBOOLEAN); }

#define setgcovalue(L,obj,x) \
  { TValue *io = (obj); GCObject *i_g=(x); \
    val_(io).gc = i_g; settt_(io, ctb(i_g->tt)); }

#define setsvalue(L,obj,x) \
  { TValue *io = (obj); TString *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(x_->tt)); \
    checkliveness(L,io); }

#define setuvalue(L,obj,x) \
  { TValue *io = (obj); Udata *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TUSERDATA)); \
    checkliveness(L,io); }

#define setthvalue(L,obj,x) \
  { TValue *io = (obj); lua_State *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TTHREAD)); \
    checkliveness(L,io); }

#define setclLvalue(L,obj,x) \
  { TValue *io = (obj); LClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TLCL)); \
    checkliveness(L,io); }

#define setclCvalue(L,obj,x) \
  { TValue *io = (obj); CClosure *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TCCL)); \
    checkliveness(L,io); }

#define sethvalue(L,obj,x) \
  { TValue *io = (obj); Table *x_ = (x); \
    val_(io).gc = obj2gco(x_); settt_(io, ctb(LUA_TTABLE)); \
    checkliveness(L,io); }

#define setdeadvalue(obj)	settt_(obj, LUA_TDEADKEY)



#define setobj(L,obj1,obj2) \
	{ TValue *io1=(obj1); *io1 = *(obj2); \
	  (void)L; checkliveness(L,io1); }


/*
** different types of assignments, according to destination
*/

/* from stack to (same) stack */
#define setobjs2s	setobj
/* to stack (not from same stack) */
#define setobj2s	setobj
#define setsvalue2s	setsvalue
#define sethvalue2s	sethvalue
#define setptvalue2s	setptvalue
/* from table to same table */
#define setobjt2t	setobj
/* to new object */
#define setobj2n	setobj
#define setsvalue2n	setsvalue

/* to table (define it as an expression to be used in macros) */
#define setobj2t(L,o1,o2)  ((void)L, *(o1)=*(o2), checkliveness(L,(o1)))

#pragma endregion



/*
** {======================================================
** types and prototypes
** =======================================================
*/


typedef TValue *StkId;  /* index to stack elements */




/*
** Header for string value; string bytes follow the end of this structure
** (aligned according to 'UTString'; see next). 这里的字符串实际存储的位置应该是在TSting对象之后紧接着的内存中(根据 getstr 这个宏定义推断，后面再确认是否是这样的)
*/
typedef struct TString {
	CommonHeader;
	lu_byte extra;  /* 保留字reserved words for short strings; "has hash" for longs */
	lu_byte shrlen;  /* length for short strings */
	unsigned int hash;
	union {
		size_t lnglen;  /* length for long strings */
		struct TString *hnext;  /* linked list for hash table */
	} u;
} TString;


/*
** Ensures that address after this type is always fully aligned.
*/
typedef union UTString {
	L_Umaxalign dummy;  /* ensures maximum alignment for strings */
	TString tsv;
} UTString;


/*
** Get the actual string (array of bytes) from a 'TString'. //从TSting对象中或者实际的字符串/字符数组
** (Access to 'extra' ensures that value is really a 'TString'.)
*/
#define getstr(ts)  \
  check_exp(sizeof((ts)->extra), cast(char *, (ts)) + sizeof(UTString)) //感觉（char* 字符串）像是在 TString的后面的内存中


/* get the actual string (array of bytes) from a Lua value */
#define svalue(o)       getstr(tsvalue(o))

/* get string length from 'TString *s' */
#define tsslen(s)	((s)->tt == LUA_TSHRSTR ? (s)->shrlen : (s)->u.lnglen)

/* get string length from 'TValue *o' */
#define vslen(o)	tsslen(tsvalue(o))


/*
** Header for userdata; memory area follows the end of this structure
** (aligned according to 'UUdata'; see next).
*/
typedef struct Udata {
	CommonHeader;
	lu_byte ttuv_;  /* user value's tag */
	struct Table *metatable;
	size_t len;  /* number of bytes 字节长短估计是Udata对象大小后的真实数据所占的字节长度*/
	union Value user_;  /* user value */
} Udata;


/*
** Ensures that address after this type is always fully aligned.
*/
typedef union UUdata {
	L_Umaxalign dummy;  /* ensures maximum alignment for 'local' udata */
	Udata uv;
} UUdata;


/*
**  Get the address of memory block inside 'Udata'.
** (Access to 'ttuv_' ensures that value is really a 'Udata'.)
*/
#define getudatamem(u)  \
  check_exp(sizeof((u)->ttuv_), (cast(char*, (u)) + sizeof(UUdata)))

#define setuservalue(L,u,o) \
	{ const TValue *io=(o); Udata *iu = (u); \
	  iu->user_ = io->value_; iu->ttuv_ = rttype(io); \
	  checkliveness(L,io); }


#define getuservalue(L,u,o) \
	{ TValue *io=(o); const Udata *iu = (u); \
	  io->value_ = iu->user_; settt_(io, iu->ttuv_); \
	  checkliveness(L,io); }


/*
** Description of an upvalue for function prototypes/原型
*/
typedef struct Upvaldesc {
	TString *name;  /* upvalue name (for debug information) */
	lu_byte instack;  /* whether it is in stack (register) */
	lu_byte idx;  /* index of upvalue (in stack or in outer function's list) */
} Upvaldesc;


/*
** Description of a local variable for function prototypes
** (used for debug information)
*/
typedef struct LocVar {
	TString *varname;
	int startpc;  /* first point where variable is active */
	int endpc;    /* first point where variable is dead */
} LocVar;


/*
** Function Prototypes
*/
typedef struct Proto {
	CommonHeader;
	lu_byte numparams;  /* number of fixed parameters */
	lu_byte is_vararg;
	lu_byte maxstacksize;  /* number of registers needed by this function */
	int sizeupvalues;  /* size of 'upvalues' */
	int sizek;  /* size of 'k' */
	int sizecode;
	int sizelineinfo;
	int sizep;  /* size of 'p' */
	int sizelocvars;
	int linedefined;  /* debug information  */
	int lastlinedefined;  /* debug information  */
	TValue *k;  /* constants used by the function */
	Instruction *code;  /* opcodes */
	struct Proto **p;  /* functions defined inside the function */
	int *lineinfo;  /* map from opcodes to source lines (debug information) */
	LocVar *locvars;  /* information about local variables (debug information) */
	Upvaldesc *upvalues;  /* upvalue information */
	struct LClosure *cache;  /* last-created closure with this prototype */
	TString  *source;  /* used for debug information */
	GCObject *gclist;
} Proto;



/*
** Lua Upvalues
*/
typedef struct UpVal UpVal;


/*
** Closures
*/

#define ClosureHeader \
	CommonHeader; lu_byte nupvalues; GCObject *gclist
//c闭包
typedef struct CClosure {
	ClosureHeader;
	lua_CFunction f;
	TValue upvalue[1];  /* list of upvalues */
} CClosure;

//lua闭包
typedef struct LClosure {
	ClosureHeader;
	struct Proto *p;
	UpVal *upvals[1];  /* list of upvalues */
} LClosure;


typedef union Closure {
	CClosure c;
	LClosure l;
} Closure;


#define isLfunction(o)	ttisLclosure(o)

#define getproto(o)	(clLvalue(o)->p)


/*
** Tables
*/
// 在任何时候TKey的类型只有两个，要么是整数，要么不是整数（非nil）
// next在之前的版本是 struct Node *next; 类型的，5.3换成了int类型，是一个偏移。next指向冲突的下一个节点
typedef union TKey {
	struct {
		TValuefields;
		int next;  /* for chaining (offset for next node) */
	} nk;
	TValue tvk;
} TKey;


/* copy a value into a key without messing up field 'next' */
#define setnodekey(L,key,obj) \
	{ TKey *k_=(key); const TValue *io_=(obj); \
	  k_->nk.value_ = io_->value_; k_->nk.tt_ = io_->tt_; \
	  (void)L; checkliveness(L,io_); }

// 此自定义的Node节点就是table的节点值
typedef struct Node {
	TValue i_val;
	TKey i_key;
} Node;

/*
	更具自己不成熟的分析，table表的hash部分并不像全局字符串的hashtable来解决冲突
	table的hash部分解决冲突应该是开放寻址法
	stringtable 解决冲突的方式应该是链接法
*/
// flags表示此table中存在哪些元方法，默认是0。元方法对应的bit定义在ltm.h中
// lsizenode是该表中以2为底的散列表大小的对数值。
// sizearray是数组部分的大小
// *array指向数组部分的指针
// *node指向散列表起始位置的指针
// *lastfree指向散列表最后位置的指针（目前感觉像是指向最后的的一个freeNode后面的一个节点）
// *metatable存放该表的元表
// *gclist：GC相关的链表
typedef struct Table {
	CommonHeader;
	lu_byte flags;  /* 1<<p means tagmethod(p) is not present */
	lu_byte lsizenode;  /* log2 of size of 'node' array */
	unsigned int sizearray;  /* size of 'array' array */
	TValue *array;  /* array part */
	Node *node;
	Node *lastfree;  /* any free position is before this position 指向最后一个node的后面一个位置*/
	struct Table *metatable;
	GCObject *gclist;
} Table;



/*
** 'module' operation for hashing (size is always a power of 2) 用来散列的模块操作
*/
//(size&(size-1))==0 判断size是否是2的指数
#define lmod(s,size) \
	(check_exp((size&(size-1))==0, (cast(int, (s) & ((size)-1)))))


#define twoto(x)	(1<<(x))
#define sizenode(t)	(twoto((t)->lsizenode))


/*
** (address of) a fixed nil value
*/
#define luaO_nilobject		(&luaO_nilobject_)


LUAI_DDEC const TValue luaO_nilobject_;

/* size of buffer for 'luaO_utf8esc' function */
#define UTF8BUFFSZ	8

LUAI_FUNC int luaO_int2fb(unsigned int x);
LUAI_FUNC int luaO_fb2int(int x);
LUAI_FUNC int luaO_utf8esc(char *buff, unsigned long x);
LUAI_FUNC int luaO_ceillog2(unsigned int x);
LUAI_FUNC void luaO_arith(lua_State *L, int op, const TValue *p1,
	const TValue *p2, TValue *res);
LUAI_FUNC size_t luaO_str2num(const char *s, TValue *o);
LUAI_FUNC int luaO_hexavalue(int c);
LUAI_FUNC void luaO_tostring(lua_State *L, StkId obj);
LUAI_FUNC const char *luaO_pushvfstring(lua_State *L, const char *fmt,
	va_list argp);
LUAI_FUNC const char *luaO_pushfstring(lua_State *L, const char *fmt, ...);
LUAI_FUNC void luaO_chunkid(char *out, const char *source, size_t len);


#endif

