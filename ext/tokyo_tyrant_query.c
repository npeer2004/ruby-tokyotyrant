#include <tokyo_tyrant_query.h>

static VALUE cQuery_initialize(VALUE vself, VALUE vrdb){
  VALUE vqry;
  TCRDB *db;
  RDBQRY *qry;
  Check_Type(vrdb, T_OBJECT);
  vrdb = rb_iv_get(vrdb, RDBVNDATA);
  Data_Get_Struct(vrdb, TCRDB, db);
  qry = tcrdbqrynew(db);
  vqry = Data_Wrap_Struct(rb_cObject, 0, tcrdbqrydel, qry);
  rb_iv_set(vself, RDBQRYVNDATA, vqry);
  rb_iv_set(vself, RDBVNDATA, vrdb);
  return Qnil;
}

static VALUE cQuery_addcond(VALUE vself, VALUE vname, VALUE vop, VALUE vexpr){
  VALUE vqry;
  RDBQRY *qry;
  vname = StringValueEx(vname);
  vexpr = StringValueEx(vexpr);

  if (TYPE(vop) == T_SYMBOL) vop = rb_str_new2(rb_id2name(SYM2ID(vop)));

  if (TYPE(vop) == T_STRING){
    vop = StringValueEx(vop);
    vop = tctdbqrystrtocondop(RSTRING_PTR(vop));
    vop = INT2NUM(vop);
  }

  vqry = rb_iv_get(vself, RDBQRYVNDATA);
  Data_Get_Struct(vqry, RDBQRY, qry);
  tcrdbqryaddcond(qry, RSTRING_PTR(vname), NUM2INT(vop), RSTRING_PTR(vexpr));
  return vself;
}

static VALUE cQuery_setorder(int argc, VALUE *argv, VALUE vself){
  VALUE vname, vtype;
  int type;
  VALUE vqry;
  RDBQRY *qry;

  rb_scan_args(argc, argv, "11", &vname, &vtype);
  if(NIL_P(vtype)) vtype = INT2NUM(RDBQOSTRASC);

  vname = StringValueEx(vname);

  switch(TYPE(vtype)){
    case T_SYMBOL:
      vtype = rb_str_new2(rb_id2name(SYM2ID(vtype)));
    case T_STRING:
      vtype = StringValueEx(vtype);
      type = tctdbqrystrtoordertype(RSTRING_PTR(vtype));
      break;
    case T_FIXNUM:
      type = NUM2INT(vtype);
      break;
    default:
      rb_raise(rb_eArgError, "type must be symbol, string or integer");
      break;
  }

  vqry = rb_iv_get(vself, RDBQRYVNDATA);
  Data_Get_Struct(vqry, RDBQRY, qry);
  tcrdbqrysetorder(qry, RSTRING_PTR(vname), type);
  return vself;
}

static VALUE cQuery_setlimit(int argc, VALUE *argv, VALUE vself){
  VALUE vqry, vmax, vskip;
  RDBQRY *qry;
  vqry = rb_iv_get(vself, RDBQRYVNDATA);
  Data_Get_Struct(vqry, RDBQRY, qry);
  rb_scan_args(argc, argv, "11", &vmax, &vskip);
  if(NIL_P(vskip)) vskip = INT2FIX(0);

  tcrdbqrysetlimit(qry, NUM2INT(vmax), NUM2INT(vskip));
  return vself;
}

static VALUE cQuery_search(VALUE vself){
  VALUE vqry, vary;
  RDBQRY *qry;
  TCLIST *res;
  vqry = rb_iv_get(vself, RDBQRYVNDATA);
  Data_Get_Struct(vqry, RDBQRY, qry);
  res = tcrdbqrysearch(qry);
  vary = listtovary(res);
  tclistdel(res);
  return vary;
}

static VALUE cQuery_searchout(VALUE vself){
  VALUE vqry;
  RDBQRY *qry;
  vqry = rb_iv_get(vself, RDBQRYVNDATA);
  Data_Get_Struct(vqry, RDBQRY, qry);
  return tcrdbqrysearchout(qry) ? Qtrue : Qfalse;
}

static VALUE cQuery_searchcount(VALUE vself){
  VALUE vqry;
  RDBQRY *qry;
  vqry = rb_iv_get(vself, RDBQRYVNDATA);
  Data_Get_Struct(vqry, RDBQRY, qry);
  return LL2NUM(tcrdbqrysearchcount(qry));
}

static VALUE cQuery_get(VALUE vself){
  int i, num, ksiz;
  const char *name, *col, *pkey;
  VALUE vqry, vary, vcols, vpkey, vname;
  RDBQRY *qry;
  TCLIST *res;
  TCMAP *cols;
  vqry = rb_iv_get(vself, RDBQRYVNDATA);
  Data_Get_Struct(vqry, RDBQRY, qry);

  res = tcrdbqrysearchget(qry);
  num = tclistnum(res);
  vary = rb_ary_new2(num);
  vpkey = rb_iv_get(vself, "@pkey");
  if(vpkey == Qnil) vpkey = rb_str_new2("__id");
  for(i = 0; i < num; i++){
    vcols = rb_hash_new();
    cols = tcrdbqryrescols(res, i);
    if(cols){
      tcmapiterinit(cols);
      while((name = tcmapiternext(cols, &ksiz)) != NULL){
        col = tcmapget2(cols, name);
        vname = ksiz == 0 ? vpkey : rb_str_new2(name);
        rb_hash_aset(vcols, vname, rb_str_new2(col));
      }
    }
    tcmapdel(cols);
    rb_ary_push(vary, vcols);
  }
  tclistdel(res);
  return vary;
}

static VALUE cQuery_set_pkey(VALUE vself, VALUE vpkey) {
  return rb_iv_set(vself, "@pkey", vpkey);
}

void init_query(){
  rb_define_const(cQuery, "CSTREQ", INT2NUM(RDBQCSTREQ));
  rb_define_const(cQuery, "CSTRINC", INT2NUM(RDBQCSTRINC));
  rb_define_const(cQuery, "CSTRBW", INT2NUM(RDBQCSTRBW));
  rb_define_const(cQuery, "CSTREW", INT2NUM(RDBQCSTREW));
  rb_define_const(cQuery, "CSTRAND", INT2NUM(RDBQCSTRAND));
  rb_define_const(cQuery, "CSTROR", INT2NUM(RDBQCSTROR));
  rb_define_const(cQuery, "CSTROREQ", INT2NUM(RDBQCSTROREQ));
  rb_define_const(cQuery, "CSTRRX", INT2NUM(RDBQCSTRRX));
  rb_define_const(cQuery, "CNUMEQ", INT2NUM(RDBQCNUMEQ));
  rb_define_const(cQuery, "CNUMGT", INT2NUM(RDBQCNUMGT));
  rb_define_const(cQuery, "CNUMGE", INT2NUM(RDBQCNUMGE));
  rb_define_const(cQuery, "CNUMLT", INT2NUM(RDBQCNUMLT));
  rb_define_const(cQuery, "CNUMLE", INT2NUM(RDBQCNUMLE));
  rb_define_const(cQuery, "CNUMBT", INT2NUM(RDBQCNUMBT));
  rb_define_const(cQuery, "CNUMOREQ", INT2NUM(RDBQCNUMOREQ));
  rb_define_const(cQuery, "CNEGATE", INT2NUM(RDBQCNEGATE));
  rb_define_const(cQuery, "CNOIDX", INT2NUM(RDBQCNOIDX));
  rb_define_const(cQuery, "OSTRASC", INT2NUM(RDBQOSTRASC));
  rb_define_const(cQuery, "OSTRDESC", INT2NUM(RDBQOSTRDESC));
  rb_define_const(cQuery, "ONUMASC", INT2NUM(RDBQONUMASC));
  rb_define_const(cQuery, "ONUMDESC", INT2NUM(RDBQONUMDESC));

  rb_define_private_method(cQuery, "initialize", cQuery_initialize, 1);
  rb_define_method(cQuery, "addcond", cQuery_addcond, 3);
  rb_define_alias(cQuery, "add_condition", "addcond");
  rb_define_alias(cQuery, "condition", "addcond");
  rb_define_alias(cQuery, "add", "addcond");                    // Rufus Compat
  rb_define_method(cQuery, "setorder", cQuery_setorder, -1);
  rb_define_alias(cQuery, "order_by", "setorder");              // Rufus Compat
  rb_define_method(cQuery, "setlimit", cQuery_setlimit, -1);
  rb_define_alias(cQuery, "setmax", "setlimit");                 // Rufus Compat
  rb_define_alias(cQuery, "limit", "setlimit");                 // Rufus Compat
  rb_define_method(cQuery, "search", cQuery_search, 0);
  rb_define_alias(cQuery, "run", "search");
  rb_define_method(cQuery, "searchout", cQuery_searchout, 0);
  rb_define_alias(cQuery, "delete", "searchout");               // Rufus Compat
  rb_define_method(cQuery, "searchcount", cQuery_searchcount, 0);
  rb_define_alias(cQuery, "count", "searchcount");              // Rufus Compat
  rb_define_method(cQuery, "get", cQuery_get, 0);
  rb_define_method(cQuery, "set_pkey", cQuery_set_pkey, 1);
  rb_define_alias(cQuery, "pkey=", "set_pkey");
}
