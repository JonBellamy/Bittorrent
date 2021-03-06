/* $Id: libnatpmpmodule.c,v 1.4 2011/01/03 17:31:03 nanard Exp $ */
/* libnatpmp
 * Copyright (c) 2007-2011, Thomas BERNARD <miniupnp@free.fr>
 * http://miniupnp.free.fr/libnatpmp.html
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */
#include <Python.h>
#ifdef WIN32
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define STATICLIB
#include "structmember.h"
#include "natpmp.h"

/* for compatibility with Python < 2.4 */
#ifndef Py_RETURN_NONE
#define Py_RETURN_NONE return Py_INCREF(Py_None), Py_None
#endif

#ifndef Py_RETURN_TRUE
#define Py_RETURN_TRUE return Py_INCREF(Py_True), Py_True
#endif

#ifndef Py_RETURN_FALSE
#define Py_RETURN_FALSE return Py_INCREF(Py_False), Py_False
#endif

typedef struct {
  PyObject_HEAD

  /* Type-specific fields go here. */
  unsigned int discoverdelay;
  
  natpmp_t natpmp;
} NATPMPObject;

static PyMemberDef NATPMP_members[] = {
  {"discoverdelay", T_UINT, offsetof(NATPMPObject, discoverdelay),
   0/*READWRITE*/, "value in ms used to wait for NATPMP responses"
  },
  {NULL}
};

static PyObject *
NATPMPObject_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  NATPMPObject *self;
  
  self = (NATPMPObject *)type->tp_alloc(type, 0);
  if (self) {
    initnatpmp(&self->natpmp, 0, 0);
  }

  return (PyObject *)self;
}

static void
NATPMPObject_dealloc(NATPMPObject *self)
{
  closenatpmp(&self->natpmp);
  self->ob_type->tp_free((PyObject*)self);
}

static PyObject *
NATPMP_externalipaddress(NATPMPObject *self)
{
  int r;
  struct timeval timeout;
  fd_set fds;
  natpmpresp_t response;
  
  r = sendpublicaddressrequest(&self->natpmp);
  
  if (r < 0) {
#ifdef ENABLE_STRNATPMPERR
    PyErr_SetString(PyExc_Exception, strnatpmperr(r));
#endif
    return NULL;
  }
    
  do {
    FD_ZERO(&fds);
    FD_SET(self->natpmp.s, &fds);
    getnatpmprequesttimeout(&self->natpmp, &timeout);
    select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
    r = readnatpmpresponseorretry(&self->natpmp, &response);
    if (r < 0 && r != NATPMP_TRYAGAIN) {
#ifdef ENABLE_STRNATPMPERR
      PyErr_SetString(PyExc_Exception, strnatpmperr(r));
#endif
      return NULL;
    }
  } while (r == NATPMP_TRYAGAIN);

  return Py_BuildValue("s", inet_ntoa(response.pnu.publicaddress.addr));
}

static PyObject *
NATPMP_domapping(natpmp_t *n, unsigned short eport, unsigned short iport, 
		 const char *protocol, unsigned int lifetime)
{
  int proto;
  struct timeval timeout;
  fd_set fds;
  natpmpresp_t response;
  int r;

  if (!strncasecmp("tcp", protocol, 3)) {
    proto = NATPMP_PROTOCOL_TCP;
  } else if (!strncasecmp("udp", protocol, 3)) {
    proto = NATPMP_PROTOCOL_UDP;
  } else {
    PyErr_SetString(PyExc_Exception, "Unknown protocol");
    return NULL;
  }

  r = sendnewportmappingrequest(n, proto, iport, eport, 
				lifetime);

  if (r < 0) {
#ifdef ENABLE_STRNATPMPERR
    PyErr_SetString(PyExc_Exception, strnatpmperr(r));
#endif
    return NULL;
  }
  
  do {
    FD_ZERO(&fds);
    FD_SET(n->s, &fds);
    getnatpmprequesttimeout(n, &timeout);
    select(FD_SETSIZE, &fds, NULL, NULL, &timeout);
    r = readnatpmpresponseorretry(n, &response);
    if (r < 0 && r != NATPMP_TRYAGAIN) {
#ifdef ENABLE_STRNATPMPERR
      PyErr_SetString(PyExc_Exception, strnatpmperr(r));
#endif
      return NULL;
    }
  } while (r == NATPMP_TRYAGAIN);

  return Py_BuildValue("H", response.pnu.newportmapping.mappedpublicport);
}


/* AddPortMapping(externalPort, protocol, internalPort, lifetime)
 * protocol is 'UDP' or 'TCP' */
static PyObject *
NATPMP_addportmapping(NATPMPObject *self, PyObject *args)
{
  unsigned short eport;
  unsigned short iport;
  unsigned int lifetime;
  const char *protocol;

  if (!PyArg_ParseTuple(args, "HsHI", &eport, &protocol, &iport, &lifetime))
    return NULL;
  
  return NATPMP_domapping(&self->natpmp, eport, iport, protocol, lifetime);
}

/* DeletePortMapping(externalPort, protocol, internalPort)
 * protocol is 'UDP' or 'TCP' */
static PyObject *
NATPMP_deleteportmapping(NATPMPObject *self, PyObject *args)
{
  unsigned short eport;
  unsigned short iport;
  const char *protocol;

  if (!PyArg_ParseTuple(args, "HsHI", &eport, &protocol, &iport))
    return NULL;
  
  return NATPMP_domapping(&self->natpmp, eport, iport, protocol, 0);
}

/* natpmp.NATPMP object Method Table */
static PyMethodDef NATPMP_methods[] = {
  {"externalipaddress", (PyCFunction)NATPMP_externalipaddress, METH_NOARGS,
   "return external IP address"
  },
  {"addportmapping", (PyCFunction)NATPMP_addportmapping, METH_VARARGS,
   "add a port mapping"
  },
  {"deleteportmapping", (PyCFunction)NATPMP_deleteportmapping, METH_VARARGS,
   "delete a port mapping"
  },
  {NULL}  /* Sentinel */
};

static PyTypeObject NATPMPType = {
  PyObject_HEAD_INIT(NULL)
  0,					/*ob_size*/
  "libnatpmp.NATPMP",			/*tp_name*/
  sizeof(NATPMPObject),			/*tp_basicsize*/
  0,					/*tp_itemsize*/
  (destructor)NATPMPObject_dealloc,	/*tp_dealloc*/
  0,					/*tp_print*/
  0,					/*tp_getattr*/
  0,					/*tp_setattr*/
  0,					/*tp_compare*/
  0,					/*tp_repr*/
  0,					/*tp_as_number*/
  0,					/*tp_as_sequence*/
  0,					/*tp_as_mapping*/
  0,					/*tp_hash */
  0,					/*tp_call*/
  0,					/*tp_str*/
  0,					/*tp_getattro*/
  0,					/*tp_setattro*/
  0,					/*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT,			/*tp_flags*/
  "NATPMP objects",			/* tp_doc */
  0,					/* tp_traverse */
  0,					/* tp_clear */
  0,					/* tp_richcompare */
  0,					/* tp_weaklistoffset */
  0,					/* tp_iter */
  0,					/* tp_iternext */
  NATPMP_methods,			/* tp_methods */
  NATPMP_members,			/* tp_members */
  0,					/* tp_getset */
  0,					/* tp_base */
  0,					/* tp_dict */
  0,					/* tp_descr_get */
  0,					/* tp_descr_set */
  0,					/* tp_dictoffset */
  0,					/* tp_init */
  0,					/* tp_alloc */
  NATPMPObject_new,			/* tp_new */
};

/* module methods */
static PyMethodDef libnatpmp_methods[] = {
    {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC
initlibnatpmp(void) 
{
  PyObject* m;
  
  if (PyType_Ready(&NATPMPType) < 0)
    return;
  
  m = Py_InitModule3("libnatpmp", libnatpmp_methods,
		     "libnatpmp module.");

  Py_INCREF(&NATPMPType);
  PyModule_AddObject(m, "NATPMP", (PyObject *)&NATPMPType);
}

