
/* POSIX module implementation */

/* This file is also used for Windows NT/MS-Win and OS/2.  In that case the
   module actually calls itself 'nt' or 'os2', not 'posix', and a few
   functions are either unimplemented or implemented differently.  The source
   assumes that for Windows NT, the macro 'MS_WINDOWS' is defined independent
   of the compiler used.  Different compilers define their own feature
   test macro, e.g. '__BORLANDC__' or '_MSC_VER'.  For OS/2, the compiler
   independent macro PYOS_OS2 should be defined.  On OS/2 the default
   compiler is assumed to be IBM's VisualAge C++ (VACPP).  PYCC_GCC is used
   as the compiler specific macro for the EMX port of gcc to OS/2. */

/* See also ../Dos/dosmodule.c */

#include "Python.h"
#include "structseq.h"

PyDoc_STRVAR(posix__doc__,
"This module provides access to operating system functionality that is\n\
standardized by the C Standard and the POSIX standard (a thinly\n\
disguised Unix interface).  Refer to the library manual and\n\
corresponding Unix manual entries for more information on calls.");

#if defined(PYOS_OS2)
#define  INCL_DOS
#define  INCL_DOSERRORS
#define  INCL_DOSPROCESS
#define  INCL_NOPMAPI
#include <os2.h>
#if defined(PYCC_GCC)
#include <ctype.h>
#include <io.h>
#include <stdio.h>
#include <process.h>
#include "osdefs.h"
#endif
#endif

#include <sys/types.h>
#include <sys/stat.h>

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>		/* For WNOHANG */
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif /* HAVE_FCNTL_H */

#ifdef HAVE_GRP_H
#include <grp.h>
#endif

/* Various compilers have only certain posix functions */
/* XXX Gosh I wish these were all moved into pyconfig.h */
#if defined(PYCC_VACPP) && defined(PYOS_OS2)
#include <process.h>
#else
#if defined(__WATCOMC__) && !defined(__QNX__)		/* Watcom compiler */
#define HAVE_GETCWD     1
#define HAVE_OPENDIR    1
#define HAVE_SYSTEM	1
#if defined(__OS2__)
#define HAVE_EXECV      1
#define HAVE_WAIT       1
#endif
#include <process.h>
#else
#ifdef __BORLANDC__		/* Borland compiler */
#define HAVE_EXECV      1
#define HAVE_GETCWD     1
#define HAVE_OPENDIR    1
#define HAVE_PIPE       1
#define HAVE_POPEN      1
#define HAVE_SYSTEM	1
#define HAVE_WAIT       1
#else
#ifdef _MSC_VER		/* Microsoft compiler */
#define HAVE_GETCWD     1
#define HAVE_SPAWNV	1
#define HAVE_EXECV      1
#define HAVE_PIPE       1
#define HAVE_POPEN      1
#define HAVE_SYSTEM	1
#define HAVE_CWAIT	1
#else
#if defined(PYOS_OS2) && defined(PYCC_GCC)
/* Everything needed is defined in PC/os2emx/pyconfig.h */
#else			/* all other compilers */
/* Unix functions that the configure script doesn't check for */
#define HAVE_EXECV      1
#define HAVE_FORK       1
#if defined(__USLC__) && defined(__SCO_VERSION__)	/* SCO UDK Compiler */
#define HAVE_FORK1      1
#endif
#define HAVE_GETCWD     1
#define HAVE_GETEGID    1
#define HAVE_GETEUID    1
#define HAVE_GETGID     1
#define HAVE_GETPPID    1
#define HAVE_GETUID     1
#define HAVE_KILL       1
#define HAVE_OPENDIR    1
#define HAVE_PIPE       1
#define HAVE_POPEN      1
#define HAVE_SYSTEM	1
#define HAVE_WAIT       1
#define HAVE_TTYNAME	1
#endif  /* PYOS_OS2 && PYCC_GCC */
#endif  /* _MSC_VER */
#endif  /* __BORLANDC__ */
#endif  /* ! __WATCOMC__ || __QNX__ */
#endif /* ! __IBMC__ */

#ifndef _MSC_VER

#if defined(sun) && !defined(__SVR4)
/* SunOS 4.1.4 doesn't have prototypes for these: */
extern int rename(const char *, const char *);
extern int pclose(FILE *);
extern int fclose(FILE *);
extern int fsync(int);
extern int lstat(const char *, struct stat *);
extern int symlink(const char *, const char *);
#endif

#if defined(__sgi)&&_COMPILER_VERSION>=700
/* declare ctermid_r if compiling with MIPSPro 7.x in ANSI C mode
   (default) */
extern char        *ctermid_r(char *);
#endif

#ifndef HAVE_UNISTD_H
#if defined(PYCC_VACPP)
extern int mkdir(char *);
#else
#if ( defined(__WATCOMC__) || defined(_MSC_VER) ) && !defined(__QNX__)
extern int mkdir(const char *);
#else
extern int mkdir(const char *, mode_t);
#endif
#endif
#if defined(__IBMC__) || defined(__IBMCPP__)
extern int chdir(char *);
extern int rmdir(char *);
#else
extern int chdir(const char *);
extern int rmdir(const char *);
#endif
#ifdef __BORLANDC__
extern int chmod(const char *, int);
#else
extern int chmod(const char *, mode_t);
#endif
extern int chown(const char *, uid_t, gid_t);
extern char *getcwd(char *, int);
extern char *strerror(int);
extern int link(const char *, const char *);
extern int rename(const char *, const char *);
extern int stat(const char *, struct stat *);
extern int unlink(const char *);
extern int pclose(FILE *);
#ifdef HAVE_SYMLINK
extern int symlink(const char *, const char *);
#endif /* HAVE_SYMLINK */
#ifdef HAVE_LSTAT
extern int lstat(const char *, struct stat *);
#endif /* HAVE_LSTAT */
#endif /* !HAVE_UNISTD_H */

#endif /* !_MSC_VER */

#ifdef HAVE_UTIME_H
#include <utime.h>
#endif /* HAVE_UTIME_H */

#ifdef HAVE_SYS_UTIME_H
#include <sys/utime.h>
#define HAVE_UTIME_H /* pretend we do for the rest of this file */
#endif /* HAVE_SYS_UTIME_H */

#ifdef HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif /* HAVE_SYS_TIMES_H */

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif /* HAVE_SYS_PARAM_H */

#ifdef HAVE_SYS_UTSNAME_H
#include <sys/utsname.h>
#endif /* HAVE_SYS_UTSNAME_H */

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#if defined(__WATCOMC__) && !defined(__QNX__)
#include <direct.h>
#define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#define dirent direct
#define NAMLEN(dirent) (dirent)->d_namlen
#endif
#ifdef HAVE_SYS_NDIR_H
#include <sys/ndir.h>
#endif
#ifdef HAVE_SYS_DIR_H
#include <sys/dir.h>
#endif
#ifdef HAVE_NDIR_H
#include <ndir.h>
#endif
#endif

#ifdef _MSC_VER
#include <direct.h>
#include <io.h>
#include <process.h>
#include "osdefs.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>	/* for ShellExecute() */
#define popen	_popen
#define pclose	_pclose
#endif /* _MSC_VER */

#if defined(PYCC_VACPP) && defined(PYOS_OS2)
#include <io.h>
#endif /* OS2 */

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif /* MAXPATHLEN */

#ifdef UNION_WAIT
/* Emulate some macros on systems that have a union instead of macros */

#ifndef WIFEXITED
#define WIFEXITED(u_wait) (!(u_wait).w_termsig && !(u_wait).w_coredump)
#endif

#ifndef WEXITSTATUS
#define WEXITSTATUS(u_wait) (WIFEXITED(u_wait)?((u_wait).w_retcode):-1)
#endif

#ifndef WTERMSIG
#define WTERMSIG(u_wait) ((u_wait).w_termsig)
#endif

#endif /* UNION_WAIT */

/* Don't use the "_r" form if we don't need it (also, won't have a
   prototype for it, at least on Solaris -- maybe others as well?). */
#if defined(HAVE_CTERMID_R) && defined(WITH_THREAD)
#define USE_CTERMID_R
#endif

#if defined(HAVE_TMPNAM_R) && defined(WITH_THREAD)
#define USE_TMPNAM_R
#endif

/* choose the appropriate stat and fstat functions and return structs */
#undef STAT
#if defined(MS_WIN64) || defined(MS_WINDOWS)
#	define STAT _stati64
#	define FSTAT _fstati64
#	define STRUCT_STAT struct _stati64
#else
#	define STAT stat
#	define FSTAT fstat
#	define STRUCT_STAT struct stat
#endif

#if defined(HAVE_MKNOD) && defined(HAVE_SYS_MKDEV_H)
#include <sys/mkdev.h>
#endif

/* Return a dictionary corresponding to the POSIX environment table */
#ifdef WITH_NEXT_FRAMEWORK
/* On Darwin/MacOSX a shared library or framework has no access to
** environ directly, we must obtain it with _NSGetEnviron().
*/
#include <crt_externs.h>
static char **environ;
#elif !defined(_MSC_VER) && ( !defined(__WATCOMC__) || defined(__QNX__) )
extern char **environ;
#endif /* !_MSC_VER */

static PyObject *
convertenviron(void)
{
	PyObject *d;
	char **e;
	d = PyDict_New();
	if (d == NULL)
		return NULL;
#ifdef WITH_NEXT_FRAMEWORK
	if (environ == NULL)
		environ = *_NSGetEnviron();
#endif
	if (environ == NULL)
		return d;
	/* This part ignores errors */
	for (e = environ; *e != NULL; e++) {
		PyObject *k;
		PyObject *v;
		char *p = strchr(*e, '=');
		if (p == NULL)
			continue;
		k = PyString_FromStringAndSize(*e, (int)(p-*e));
		if (k == NULL) {
			PyErr_Clear();
			continue;
		}
		v = PyString_FromString(p+1);
		if (v == NULL) {
			PyErr_Clear();
			Py_DECREF(k);
			continue;
		}
		if (PyDict_GetItem(d, k) == NULL) {
			if (PyDict_SetItem(d, k, v) != 0)
				PyErr_Clear();
		}
		Py_DECREF(k);
		Py_DECREF(v);
	}
#if defined(PYOS_OS2)
    {
        APIRET rc;
        char   buffer[1024]; /* OS/2 Provides a Documented Max of 1024 Chars */

        rc = DosQueryExtLIBPATH(buffer, BEGIN_LIBPATH);
	if (rc == NO_ERROR) { /* (not a type, envname is NOT 'BEGIN_LIBPATH') */
            PyObject *v = PyString_FromString(buffer);
		    PyDict_SetItemString(d, "BEGINLIBPATH", v);
            Py_DECREF(v);
        }
        rc = DosQueryExtLIBPATH(buffer, END_LIBPATH);
        if (rc == NO_ERROR) { /* (not a typo, envname is NOT 'END_LIBPATH') */
            PyObject *v = PyString_FromString(buffer);
		    PyDict_SetItemString(d, "ENDLIBPATH", v);
            Py_DECREF(v);
        }
    }
#endif
	return d;
}


/* Set a POSIX-specific error from errno, and return NULL */

static PyObject *
posix_error(void)
{
	return PyErr_SetFromErrno(PyExc_OSError);
}
static PyObject *
posix_error_with_filename(char* name)
{
	return PyErr_SetFromErrnoWithFilename(PyExc_OSError, name);
}

#ifdef Py_WIN_WIDE_FILENAMES
static PyObject *
posix_error_with_unicode_filename(Py_UNICODE* name)
{
	return PyErr_SetFromErrnoWithUnicodeFilename(PyExc_OSError, name);
}
#endif /* Py_WIN_WIDE_FILENAMES */


static PyObject *
posix_error_with_allocated_filename(char* name)
{
	PyObject *rc = PyErr_SetFromErrnoWithFilename(PyExc_OSError, name);
	PyMem_Free(name);
	return rc;
}

#ifdef MS_WINDOWS
static PyObject *
win32_error(char* function, char* filename)
{
	/* XXX We should pass the function name along in the future.
	   (_winreg.c also wants to pass the function name.)
	   This would however require an additional param to the
	   Windows error object, which is non-trivial.
	*/
	errno = GetLastError();
	if (filename)
		return PyErr_SetFromWindowsErrWithFilename(errno, filename);
	else
		return PyErr_SetFromWindowsErr(errno);
}

#ifdef Py_WIN_WIDE_FILENAMES
static PyObject *
win32_error_unicode(char* function, Py_UNICODE* filename)
{
	/* XXX - see win32_error for comments on 'function' */
	errno = GetLastError();
	if (filename)
		return PyErr_SetFromWindowsErrWithUnicodeFilename(errno, filename);
	else
		return PyErr_SetFromWindowsErr(errno);
}

static PyObject *_PyUnicode_FromFileSystemEncodedObject(register PyObject *obj)
{
	/* XXX Perhaps we should make this API an alias of
	   PyObject_Unicode() instead ?! */
	if (PyUnicode_CheckExact(obj)) {
		Py_INCREF(obj);
		return obj;
	}
	if (PyUnicode_Check(obj)) {
		/* For a Unicode subtype that's not a Unicode object,
		   return a true Unicode object with the same data. */
	return PyUnicode_FromUnicode(PyUnicode_AS_UNICODE(obj),
	                             PyUnicode_GET_SIZE(obj));
	}
	return PyUnicode_FromEncodedObject(obj, 
	                                   Py_FileSystemDefaultEncoding, 
	                                   "strict");
}

#endif /* Py_WIN_WIDE_FILENAMES */

#endif

#if defined(PYOS_OS2)
/**********************************************************************
 *         Helper Function to Trim and Format OS/2 Messages
 **********************************************************************/
    static void
os2_formatmsg(char *msgbuf, int msglen, char *reason)
{
    msgbuf[msglen] = '\0'; /* OS/2 Doesn't Guarantee a Terminator */

    if (strlen(msgbuf) > 0) { /* If Non-Empty Msg, Trim CRLF */
        char *lastc = &msgbuf[ strlen(msgbuf)-1 ];

        while (lastc > msgbuf && isspace(*lastc))
            *lastc-- = '\0'; /* Trim Trailing Whitespace (CRLF) */
    }

    /* Add Optional Reason Text */
    if (reason) {
        strcat(msgbuf, " : ");
        strcat(msgbuf, reason);
    }
}

/**********************************************************************
 *             Decode an OS/2 Operating System Error Code
 *
 * A convenience function to lookup an OS/2 error code and return a
 * text message we can use to raise a Python exception.
 *
 * Notes:
 *   The messages for errors returned from the OS/2 kernel reside in
 *   the file OSO001.MSG in the \OS2 directory hierarchy.
 *
 **********************************************************************/
    static char *
os2_strerror(char *msgbuf, int msgbuflen, int errorcode, char *reason)
{
    APIRET rc;
    ULONG  msglen;

    /* Retrieve Kernel-Related Error Message from OSO001.MSG File */
    Py_BEGIN_ALLOW_THREADS
    rc = DosGetMessage(NULL, 0, msgbuf, msgbuflen,
                       errorcode, "oso001.msg", &msglen);
    Py_END_ALLOW_THREADS

    if (rc == NO_ERROR)
        os2_formatmsg(msgbuf, msglen, reason);
    else
        PyOS_snprintf(msgbuf, msgbuflen,
        	      "unknown OS error #%d", errorcode);

    return msgbuf;
}

/* Set an OS/2-specific error and return NULL.  OS/2 kernel
   errors are not in a global variable e.g. 'errno' nor are
   they congruent with posix error numbers. */

static PyObject * os2_error(int code)
{
    char text[1024];
    PyObject *v;

    os2_strerror(text, sizeof(text), code, "");

    v = Py_BuildValue("(is)", code, text);
    if (v != NULL) {
        PyErr_SetObject(PyExc_OSError, v);
        Py_DECREF(v);
    }
    return NULL; /* Signal to Python that an Exception is Pending */
}

#endif /* OS2 */

/* POSIX generic methods */

static PyObject *
posix_fildes(PyObject *fdobj, int (*func)(int))
{
	int fd;
	int res;
	fd = PyObject_AsFileDescriptor(fdobj);
	if (fd < 0)
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = (*func)(fd);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}

#ifdef Py_WIN_WIDE_FILENAMES
static int 
unicode_file_names(void)
{
	static int canusewide = -1;
	if (canusewide == -1) {
		/* As per doc for ::GetVersion(), this is the correct test for 
		   the Windows NT family. */
		canusewide = (GetVersion() < 0x80000000) ? 1 : 0;
	}
	return canusewide;
}
#endif
  
static PyObject *
posix_1str(PyObject *args, char *format, int (*func)(const char*),
	   char *wformat, int (*wfunc)(const Py_UNICODE*))
{
	char *path1 = NULL;
	int res;
#ifdef Py_WIN_WIDE_FILENAMES
	if (unicode_file_names()) {
		PyUnicodeObject *po;
		if (PyArg_ParseTuple(args, wformat, &po)) {
			Py_BEGIN_ALLOW_THREADS
			/*  PyUnicode_AS_UNICODE OK without thread
			    lock as it is a simple dereference. */
			res = (*wfunc)(PyUnicode_AS_UNICODE(po));
			Py_END_ALLOW_THREADS
			if (res < 0)
				return posix_error_with_unicode_filename(PyUnicode_AS_UNICODE(po));
			Py_INCREF(Py_None);
			return Py_None;
		}
		/* Drop the argument parsing error as narrow
		   strings are also valid. */
		PyErr_Clear();
	}
#else
	/* Platforms that don't support Unicode filenames
	   shouldn't be passing these extra params */
	assert(wformat==NULL && wfunc == NULL);
#endif

	if (!PyArg_ParseTuple(args, format,
	                      Py_FileSystemDefaultEncoding, &path1))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = (*func)(path1);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return posix_error_with_allocated_filename(path1);
	PyMem_Free(path1);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
posix_2str(PyObject *args, 
	   char *format,
	   int (*func)(const char *, const char *),
	   char *wformat,
	   int (*wfunc)(const Py_UNICODE *, const Py_UNICODE *))
{
	char *path1 = NULL, *path2 = NULL;
	int res;
#ifdef Py_WIN_WIDE_FILENAMES
	if (unicode_file_names()) {
		PyObject *po1;
		PyObject *po2;
		if (PyArg_ParseTuple(args, wformat, &po1, &po2)) {
			if (PyUnicode_Check(po1) || PyUnicode_Check(po2)) {
				PyObject *wpath1;
				PyObject *wpath2;
				wpath1 = _PyUnicode_FromFileSystemEncodedObject(po1);
				wpath2 = _PyUnicode_FromFileSystemEncodedObject(po2);
				if (!wpath1 || !wpath2) {
					Py_XDECREF(wpath1);
					Py_XDECREF(wpath2);
					return NULL;
				}
				Py_BEGIN_ALLOW_THREADS
				/* PyUnicode_AS_UNICODE OK without thread
				   lock as it is a simple dereference.  */
				res = (*wfunc)(PyUnicode_AS_UNICODE(wpath1),
					       PyUnicode_AS_UNICODE(wpath2));
				Py_END_ALLOW_THREADS
				Py_XDECREF(wpath1);
				Py_XDECREF(wpath2);
				if (res != 0)
					return posix_error();
				Py_INCREF(Py_None);
				return Py_None;
			}
			/* Else flow through as neither is Unicode. */
		}
		/* Drop the argument parsing error as narrow
		   strings are also valid. */
		PyErr_Clear();
	}
#else
	/* Platforms that don't support Unicode filenames
	   shouldn't be passing these extra params */
	assert(wformat==NULL && wfunc == NULL);
#endif

	if (!PyArg_ParseTuple(args, format,
	                      Py_FileSystemDefaultEncoding, &path1,
	                      Py_FileSystemDefaultEncoding, &path2))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = (*func)(path1, path2);
	Py_END_ALLOW_THREADS
	PyMem_Free(path1);
	PyMem_Free(path2);
	if (res != 0)
		/* XXX how to report both path1 and path2??? */
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}

PyDoc_STRVAR(stat_result__doc__,
"stat_result: Result from stat or lstat.\n\n\
This object may be accessed either as a tuple of\n\
  (mode, ino, dev, nlink, uid, gid, size, atime, mtime, ctime)\n\
or via the attributes st_mode, st_ino, st_dev, st_nlink, st_uid, and so on.\n\
\n\
Posix/windows: If your platform supports st_blksize, st_blocks, or st_rdev,\n\
they are available as attributes only.\n\
\n\
See os.stat for more information.");

static PyStructSequence_Field stat_result_fields[] = {
	{"st_mode",    "protection bits"},
	{"st_ino",     "inode"},
	{"st_dev",     "device"},
	{"st_nlink",   "number of hard links"},
	{"st_uid",     "user ID of owner"},
	{"st_gid",     "group ID of owner"},
	{"st_size",    "total size, in bytes"},
	{"st_atime",   "time of last access"},
	{"st_mtime",   "time of last modification"},
	{"st_ctime",   "time of last change"},
#ifdef HAVE_ST_BLKSIZE
	{"st_blksize", "blocksize for filesystem I/O"},
#endif
#ifdef HAVE_ST_BLOCKS
	{"st_blocks",  "number of blocks allocated"},
#endif
#ifdef HAVE_ST_RDEV
	{"st_rdev",    "device type (if inode device)"},
#endif
	{0}
};

#ifdef HAVE_ST_BLKSIZE
#define ST_BLKSIZE_IDX 10
#else
#define ST_BLKSIZE_IDX 9
#endif

#ifdef HAVE_ST_BLOCKS
#define ST_BLOCKS_IDX (ST_BLKSIZE_IDX+1)
#else
#define ST_BLOCKS_IDX ST_BLKSIZE_IDX
#endif

#ifdef HAVE_ST_RDEV
#define ST_RDEV_IDX (ST_BLOCKS_IDX+1)
#else
#define ST_RDEV_IDX ST_BLOCKS_IDX
#endif

static PyStructSequence_Desc stat_result_desc = {
	"stat_result", /* name */
	stat_result__doc__, /* doc */
	stat_result_fields,
	10
};

PyDoc_STRVAR(statvfs_result__doc__,
"statvfs_result: Result from statvfs or fstatvfs.\n\n\
This object may be accessed either as a tuple of\n\
  (bsize, frsize, blocks, bfree, bavail, files, ffree, favail, flag, namemax),\n\
or via the attributes f_bsize, f_frsize, f_blocks, f_bfree, and so on.\n\
\n\
See os.statvfs for more information.");

static PyStructSequence_Field statvfs_result_fields[] = {
        {"f_bsize",  },
        {"f_frsize", },
        {"f_blocks", },
        {"f_bfree",  },
        {"f_bavail", },
        {"f_files",  },
        {"f_ffree",  },
        {"f_favail", },
        {"f_flag",   },
        {"f_namemax",},
        {0}
};

static PyStructSequence_Desc statvfs_result_desc = {
	"statvfs_result", /* name */
	statvfs_result__doc__, /* doc */
	statvfs_result_fields,
	10
};

static PyTypeObject StatResultType;
static PyTypeObject StatVFSResultType;

static void
fill_time(PyObject *v, int index, time_t sec, unsigned long nsec)
{
	PyObject *val;
        val = PyFloat_FromDouble(sec + 1e-9*nsec);
	PyStructSequence_SET_ITEM(v, index, val);
}

/* pack a system stat C structure into the Python stat tuple
   (used by posix_stat() and posix_fstat()) */
static PyObject*
_pystat_fromstructstat(STRUCT_STAT st)
{
	unsigned long ansec, mnsec, cnsec;
	PyObject *v = PyStructSequence_New(&StatResultType);
	if (v == NULL)
		return NULL;

        PyStructSequence_SET_ITEM(v, 0, PyInt_FromLong((long)st.st_mode));
#ifdef HAVE_LARGEFILE_SUPPORT
        PyStructSequence_SET_ITEM(v, 1,
				  PyLong_FromLongLong((LONG_LONG)st.st_ino));
#else
        PyStructSequence_SET_ITEM(v, 1, PyInt_FromLong((long)st.st_ino));
#endif
#if defined(HAVE_LONG_LONG) && !defined(MS_WINDOWS)
        PyStructSequence_SET_ITEM(v, 2,
				  PyLong_FromLongLong((LONG_LONG)st.st_dev));
#else
        PyStructSequence_SET_ITEM(v, 2, PyInt_FromLong((long)st.st_dev));
#endif
        PyStructSequence_SET_ITEM(v, 3, PyInt_FromLong((long)st.st_nlink));
        PyStructSequence_SET_ITEM(v, 4, PyInt_FromLong((long)st.st_uid));
        PyStructSequence_SET_ITEM(v, 5, PyInt_FromLong((long)st.st_gid));
#ifdef HAVE_LARGEFILE_SUPPORT
        PyStructSequence_SET_ITEM(v, 6,
				  PyLong_FromLongLong((LONG_LONG)st.st_size));
#else
        PyStructSequence_SET_ITEM(v, 6, PyInt_FromLong(st.st_size));
#endif

#ifdef HAVE_STAT_TV_NSEC
	ansec = st.st_atim.tv_nsec;
	mnsec = st.st_mtim.tv_nsec;
	cnsec = st.st_ctim.tv_nsec;
#else
	ansec = mnsec = cnsec = 0;
#endif
	fill_time(v, 7, st.st_atime, ansec);
	fill_time(v, 8, st.st_mtime, mnsec);
	fill_time(v, 9, st.st_ctime, cnsec);

#ifdef HAVE_ST_BLKSIZE
	PyStructSequence_SET_ITEM(v, ST_BLKSIZE_IDX,
			 PyInt_FromLong((long)st.st_blksize));
#endif
#ifdef HAVE_ST_BLOCKS
	PyStructSequence_SET_ITEM(v, ST_BLOCKS_IDX,
			 PyInt_FromLong((long)st.st_blocks));
#endif
#ifdef HAVE_ST_RDEV
	PyStructSequence_SET_ITEM(v, ST_RDEV_IDX,
			 PyInt_FromLong((long)st.st_rdev));
#endif

	if (PyErr_Occurred()) {
		Py_DECREF(v);
		return NULL;
	}

	return v;
}

static PyObject *
posix_do_stat(PyObject *self, PyObject *args, 
	      char *format,
	      int (*statfunc)(const char *, STRUCT_STAT *),
	      char *wformat,
	      int (*wstatfunc)(const Py_UNICODE *, STRUCT_STAT *))
{
	STRUCT_STAT st;
	char *path = NULL;	/* pass this to stat; do not free() it */
	char *pathfree = NULL;  /* this memory must be free'd */
	int res;

#ifdef MS_WINDOWS
	int pathlen;
	char pathcopy[MAX_PATH];
#endif /* MS_WINDOWS */


#ifdef Py_WIN_WIDE_FILENAMES
	/* If on wide-character-capable OS see if argument
	   is Unicode and if so use wide API.  */
	if (unicode_file_names()) {
		PyUnicodeObject *po;
		if (PyArg_ParseTuple(args, wformat, &po)) {
			Py_UNICODE wpath[MAX_PATH+1];
			pathlen = wcslen(PyUnicode_AS_UNICODE(po));
			/* the library call can blow up if the file name is too long! */
			if (pathlen > MAX_PATH) {
				errno = ENAMETOOLONG;
				return posix_error();
			}
			wcscpy(wpath, PyUnicode_AS_UNICODE(po));
			/* Remove trailing slash or backslash, unless it's the current
			   drive root (/ or \) or a specific drive's root (like c:\ or c:/).
			*/
			if (pathlen > 0 &&
				(wpath[pathlen-1]== L'\\' || wpath[pathlen-1] == L'/')) {
	    			/* It does end with a slash -- exempt the root drive cases. */
	    			/* XXX UNC root drives should also be exempted? */
				if (pathlen == 1 || (pathlen == 3 && wpath[1] == L':'))
	    			/* leave it alone */;
	    		else {
					/* nuke the trailing backslash */
					wpath[pathlen-1] = L'\0';
				}
			}
			Py_BEGIN_ALLOW_THREADS
				/* PyUnicode_AS_UNICODE result OK without
				   thread lock as it is a simple dereference. */
			res = wstatfunc(wpath, &st);
			Py_END_ALLOW_THREADS
			if (res != 0)
				return posix_error_with_unicode_filename(wpath);
			return _pystat_fromstructstat(st);
		}
		/* Drop the argument parsing error as narrow strings
		   are also valid. */
		PyErr_Clear();
	}
#endif

	if (!PyArg_ParseTuple(args, format,
	                      Py_FileSystemDefaultEncoding, &path))
		return NULL;
	pathfree = path;

#ifdef MS_WINDOWS
	pathlen = strlen(path);
	/* the library call can blow up if the file name is too long! */
	if (pathlen > MAX_PATH) {
		PyMem_Free(pathfree);
		errno = ENAMETOOLONG;
		return posix_error();
	}

	/* Remove trailing slash or backslash, unless it's the current
	   drive root (/ or \) or a specific drive's root (like c:\ or c:/).
	*/
	if (pathlen > 0 &&
	    (path[pathlen-1]== '\\' || path[pathlen-1] == '/')) {
	    	/* It does end with a slash -- exempt the root drive cases. */
	    	/* XXX UNC root drives should also be exempted? */
	    	if (pathlen == 1 || (pathlen == 3 && path[1] == ':'))
	    		/* leave it alone */;
	    	else {
			/* nuke the trailing backslash */
			strncpy(pathcopy, path, pathlen);
			pathcopy[pathlen-1] = '\0';
			path = pathcopy;
		}
	}
#endif /* MS_WINDOWS */

	Py_BEGIN_ALLOW_THREADS
	res = (*statfunc)(path, &st);
	Py_END_ALLOW_THREADS
	if (res != 0)
		return posix_error_with_allocated_filename(pathfree);

	PyMem_Free(pathfree);
	return _pystat_fromstructstat(st);
}


/* POSIX methods */

PyDoc_STRVAR(posix_access__doc__,
"access(path, mode) -> 1 if granted, 0 otherwise\n\n\
Use the real uid/gid to test for access to a path.  Note that most\n\
operations will use the effective uid/gid, therefore this routine can\n\
be used in a suid/sgid environment to test if the invoking user has the\n\
specified access to the path.  The mode argument can be F_OK to test\n\
existence, or the inclusive-OR of R_OK, W_OK, and X_OK.");

static PyObject *
posix_access(PyObject *self, PyObject *args)
{
	char *path;
	int mode;
	int res;

	if (!PyArg_ParseTuple(args, "si:access", &path, &mode))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = access(path, mode);
	Py_END_ALLOW_THREADS
	return(PyBool_FromLong(res == 0));
}

#ifndef F_OK
#define F_OK 0
#endif
#ifndef R_OK
#define R_OK 4
#endif
#ifndef W_OK
#define W_OK 2
#endif
#ifndef X_OK
#define X_OK 1
#endif

#ifdef HAVE_TTYNAME
PyDoc_STRVAR(posix_ttyname__doc__,
"ttyname(fd) -> string\n\n\
Return the name of the terminal device connected to 'fd'.");

static PyObject *
posix_ttyname(PyObject *self, PyObject *args)
{
	int id;
	char *ret;

	if (!PyArg_ParseTuple(args, "i:ttyname", &id))
		return NULL;

	ret = ttyname(id);
	if (ret == NULL)
		return(posix_error());
	return(PyString_FromString(ret));
}
#endif

#ifdef HAVE_CTERMID
PyDoc_STRVAR(posix_ctermid__doc__,
"ctermid() -> string\n\n\
Return the name of the controlling terminal for this process.");

static PyObject *
posix_ctermid(PyObject *self, PyObject *args)
{
        char *ret;
        char buffer[L_ctermid];

	if (!PyArg_ParseTuple(args, ":ctermid"))
		return NULL;

#ifdef USE_CTERMID_R
	ret = ctermid_r(buffer);
#else
        ret = ctermid(buffer);
#endif
	if (ret == NULL)
		return(posix_error());
	return(PyString_FromString(buffer));
}
#endif

PyDoc_STRVAR(posix_chdir__doc__,
"chdir(path)\n\n\
Change the current working directory to the specified path.");

static PyObject *
posix_chdir(PyObject *self, PyObject *args)
{
#ifdef MS_WINDOWS
	return posix_1str(args, "et:chdir", chdir, "U:chdir", _wchdir);
#elif defined(PYOS_OS2) && defined(PYCC_GCC)
	return posix_1str(args, "et:chdir", _chdir2, NULL, NULL);
#else
	return posix_1str(args, "et:chdir", chdir, NULL, NULL);
#endif
}

#ifdef HAVE_FCHDIR
PyDoc_STRVAR(posix_fchdir__doc__,
"fchdir(fildes)\n\n\
Change to the directory of the given file descriptor.  fildes must be\n\
opened on a directory, not a file.");

static PyObject *
posix_fchdir(PyObject *self, PyObject *fdobj)
{
	return posix_fildes(fdobj, fchdir);
}
#endif /* HAVE_FCHDIR */


PyDoc_STRVAR(posix_chmod__doc__,
"chmod(path, mode)\n\n\
Change the access permissions of a file.");

static PyObject *
posix_chmod(PyObject *self, PyObject *args)
{
	char *path = NULL;
	int i;
	int res;
	if (!PyArg_ParseTuple(args, "eti", Py_FileSystemDefaultEncoding,
	                      &path, &i))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = chmod(path, i);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return posix_error_with_allocated_filename(path);
	PyMem_Free(path);
	Py_INCREF(Py_None);
	return Py_None;
}


#ifdef HAVE_CHROOT
PyDoc_STRVAR(posix_chroot__doc__,
"chroot(path)\n\n\
Change root directory to path.");

static PyObject *
posix_chroot(PyObject *self, PyObject *args)
{
	return posix_1str(args, "et:chroot", chroot, NULL, NULL);
}
#endif

#ifdef HAVE_FSYNC
PyDoc_STRVAR(posix_fsync__doc__,
"fsync(fildes)\n\n\
force write of file with filedescriptor to disk.");

static PyObject *
posix_fsync(PyObject *self, PyObject *fdobj)
{
       return posix_fildes(fdobj, fsync);
}
#endif /* HAVE_FSYNC */

#ifdef HAVE_FDATASYNC

#ifdef __hpux
extern int fdatasync(int); /* On HP-UX, in libc but not in unistd.h */
#endif

PyDoc_STRVAR(posix_fdatasync__doc__,
"fdatasync(fildes)\n\n\
force write of file with filedescriptor to disk.\n\
 does not force update of metadata.");

static PyObject *
posix_fdatasync(PyObject *self, PyObject *fdobj)
{
       return posix_fildes(fdobj, fdatasync);
}
#endif /* HAVE_FDATASYNC */


#ifdef HAVE_CHOWN
PyDoc_STRVAR(posix_chown__doc__,
"chown(path, uid, gid)\n\n\
Change the owner and group id of path to the numeric uid and gid.");

static PyObject *
posix_chown(PyObject *self, PyObject *args)
{
	char *path = NULL;
	int uid, gid;
	int res;
	if (!PyArg_ParseTuple(args, "etii:chown",
	                      Py_FileSystemDefaultEncoding, &path,
	                      &uid, &gid))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = chown(path, (uid_t) uid, (gid_t) gid);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return posix_error_with_allocated_filename(path);
	PyMem_Free(path);
	Py_INCREF(Py_None);
	return Py_None;
}
#endif /* HAVE_CHOWN */

#ifdef HAVE_LCHOWN
PyDoc_STRVAR(posix_lchown__doc__,
"lchown(path, uid, gid)\n\n\
Change the owner and group id of path to the numeric uid and gid.\n\
This function will not follow symbolic links.");

static PyObject *
posix_lchown(PyObject *self, PyObject *args)
{
	char *path = NULL;
	int uid, gid;
	int res;
	if (!PyArg_ParseTuple(args, "etii:lchown",
	                      Py_FileSystemDefaultEncoding, &path,
	                      &uid, &gid))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = lchown(path, (uid_t) uid, (gid_t) gid);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return posix_error_with_allocated_filename(path);
	PyMem_Free(path);
	Py_INCREF(Py_None);
	return Py_None;
}
#endif /* HAVE_LCHOWN */


#ifdef HAVE_GETCWD
PyDoc_STRVAR(posix_getcwd__doc__,
"getcwd() -> path\n\n\
Return a string representing the current working directory.");

static PyObject *
posix_getcwd(PyObject *self, PyObject *args)
{
	char buf[1026];
	char *res;
	if (!PyArg_ParseTuple(args, ":getcwd"))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
#if defined(PYOS_OS2) && defined(PYCC_GCC)
	res = _getcwd2(buf, sizeof buf);
#else
	res = getcwd(buf, sizeof buf);
#endif
	Py_END_ALLOW_THREADS
	if (res == NULL)
		return posix_error();
	return PyString_FromString(buf);
}

PyDoc_STRVAR(posix_getcwdu__doc__,
"getcwdu() -> path\n\n\
Return a unicode string representing the current working directory.");

static PyObject *
posix_getcwdu(PyObject *self, PyObject *args)
{
	char buf[1026];
	char *res;
	if (!PyArg_ParseTuple(args, ":getcwd"))
		return NULL;

#ifdef Py_WIN_WIDE_FILENAMES
	if (unicode_file_names()) {
		wchar_t *wres;
		wchar_t wbuf[1026];
		Py_BEGIN_ALLOW_THREADS
		wres = _wgetcwd(wbuf, sizeof wbuf/ sizeof wbuf[0]);
		Py_END_ALLOW_THREADS
		if (wres == NULL)
			return posix_error();
		return PyUnicode_FromWideChar(wbuf, wcslen(wbuf));
	}
#endif

	Py_BEGIN_ALLOW_THREADS
#if defined(PYOS_OS2) && defined(PYCC_GCC)
	res = _getcwd2(buf, sizeof buf);
#else
	res = getcwd(buf, sizeof buf);
#endif
	Py_END_ALLOW_THREADS
	if (res == NULL)
		return posix_error();
	return PyUnicode_Decode(buf, strlen(buf), Py_FileSystemDefaultEncoding,"strict");
}
#endif


#ifdef HAVE_LINK
PyDoc_STRVAR(posix_link__doc__,
"link(src, dst)\n\n\
Create a hard link to a file.");

static PyObject *
posix_link(PyObject *self, PyObject *args)
{
	return posix_2str(args, "etet:link", link, NULL, NULL);
}
#endif /* HAVE_LINK */


PyDoc_STRVAR(posix_listdir__doc__,
"listdir(path) -> list_of_strings\n\n\
Return a list containing the names of the entries in the directory.\n\
\n\
	path: path of directory to list\n\
\n\
The list is in arbitrary order.  It does not include the special\n\
entries '.' and '..' even if they are present in the directory.");

static PyObject *
posix_listdir(PyObject *self, PyObject *args)
{
	/* XXX Should redo this putting the (now four) versions of opendir
	   in separate files instead of having them all here... */
#if defined(MS_WINDOWS) && !defined(HAVE_OPENDIR)

	PyObject *d, *v;
	HANDLE hFindFile;
	WIN32_FIND_DATA FileData;
	/* MAX_PATH characters could mean a bigger encoded string */
	char namebuf[MAX_PATH*2+5];
	char *bufptr = namebuf;
	int len = sizeof(namebuf)/sizeof(namebuf[0]);

#ifdef Py_WIN_WIDE_FILENAMES
	/* If on wide-character-capable OS see if argument
	   is Unicode and if so use wide API.  */
	if (unicode_file_names()) {
		PyUnicodeObject *po;
		if (PyArg_ParseTuple(args, "U:listdir", &po)) {
			WIN32_FIND_DATAW wFileData;
			Py_UNICODE wnamebuf[MAX_PATH*2+5];
			Py_UNICODE wch;
			wcsncpy(wnamebuf, PyUnicode_AS_UNICODE(po), MAX_PATH);
			wnamebuf[MAX_PATH] = L'\0';
			len = wcslen(wnamebuf);
			wch = (len > 0) ? wnamebuf[len-1] : L'\0';
			if (wch != L'/' && wch != L'\\' && wch != L':')
				wnamebuf[len++] = L'/';
			wcscpy(wnamebuf + len, L"*.*");
			if ((d = PyList_New(0)) == NULL)
				return NULL;
			hFindFile = FindFirstFileW(wnamebuf, &wFileData);
			if (hFindFile == INVALID_HANDLE_VALUE) {
				errno = GetLastError();
				if (errno == ERROR_FILE_NOT_FOUND) {
					return d;
				}
				Py_DECREF(d);
				return win32_error_unicode("FindFirstFileW", wnamebuf);
			}
			do {
				if (wFileData.cFileName[0] == L'.' &&
					(wFileData.cFileName[1] == L'\0' ||
					 wFileData.cFileName[1] == L'.' &&
					 wFileData.cFileName[2] == L'\0'))
					continue;
				v = PyUnicode_FromUnicode(wFileData.cFileName, wcslen(wFileData.cFileName));
				if (v == NULL) {
					Py_DECREF(d);
					d = NULL;
					break;
				}
				if (PyList_Append(d, v) != 0) {
					Py_DECREF(v);
					Py_DECREF(d);
					d = NULL;
					break;
				}
				Py_DECREF(v);
			} while (FindNextFileW(hFindFile, &wFileData) == TRUE);

			if (FindClose(hFindFile) == FALSE) {
				Py_DECREF(d);
				return win32_error_unicode("FindClose", wnamebuf);
			}
			return d;
		}
		/* Drop the argument parsing error as narrow strings
		   are also valid. */
		PyErr_Clear();
	}
#endif

	if (!PyArg_ParseTuple(args, "et#:listdir",
	                      Py_FileSystemDefaultEncoding, &bufptr, &len))
		return NULL;
	if (len > 0) {
		char ch = namebuf[len-1];
		if (ch != SEP && ch != ALTSEP && ch != ':')
			namebuf[len++] = '/';
	}
	strcpy(namebuf + len, "*.*");

	if ((d = PyList_New(0)) == NULL)
		return NULL;

	hFindFile = FindFirstFile(namebuf, &FileData);
	if (hFindFile == INVALID_HANDLE_VALUE) {
		errno = GetLastError();
		if (errno == ERROR_FILE_NOT_FOUND)
			return d;
		Py_DECREF(d);
		return win32_error("FindFirstFile", namebuf);
	}
	do {
		if (FileData.cFileName[0] == '.' &&
		    (FileData.cFileName[1] == '\0' ||
		     FileData.cFileName[1] == '.' &&
		     FileData.cFileName[2] == '\0'))
			continue;
		v = PyString_FromString(FileData.cFileName);
		if (v == NULL) {
			Py_DECREF(d);
			d = NULL;
			break;
		}
		if (PyList_Append(d, v) != 0) {
			Py_DECREF(v);
			Py_DECREF(d);
			d = NULL;
			break;
		}
		Py_DECREF(v);
	} while (FindNextFile(hFindFile, &FileData) == TRUE);

	if (FindClose(hFindFile) == FALSE) {
		Py_DECREF(d);
		return win32_error("FindClose", namebuf);
	}

	return d;

#elif defined(PYOS_OS2)

#ifndef MAX_PATH
#define MAX_PATH    CCHMAXPATH
#endif
    char *name, *pt;
    int len;
    PyObject *d, *v;
    char namebuf[MAX_PATH+5];
    HDIR  hdir = 1;
    ULONG srchcnt = 1;
    FILEFINDBUF3   ep;
    APIRET rc;

    if (!PyArg_ParseTuple(args, "t#:listdir", &name, &len))
        return NULL;
    if (len >= MAX_PATH) {
		PyErr_SetString(PyExc_ValueError, "path too long");
        return NULL;
    }
    strcpy(namebuf, name);
    for (pt = namebuf; *pt; pt++)
        if (*pt == ALTSEP)
            *pt = SEP;
    if (namebuf[len-1] != SEP)
        namebuf[len++] = SEP;
    strcpy(namebuf + len, "*.*");

	if ((d = PyList_New(0)) == NULL)
        return NULL;

    rc = DosFindFirst(namebuf,         /* Wildcard Pattern to Match */
                      &hdir,           /* Handle to Use While Search Directory */
                      FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_DIRECTORY,
                      &ep, sizeof(ep), /* Structure to Receive Directory Entry */
                      &srchcnt,        /* Max and Actual Count of Entries Per Iteration */
                      FIL_STANDARD);   /* Format of Entry (EAs or Not) */

    if (rc != NO_ERROR) {
        errno = ENOENT;
        return posix_error_with_filename(name);
    }

    if (srchcnt > 0) { /* If Directory is NOT Totally Empty, */
        do {
            if (ep.achName[0] == '.'
            && (ep.achName[1] == '\0' || (ep.achName[1] == '.' && ep.achName[2] == '\0')))
                continue; /* Skip Over "." and ".." Names */

            strcpy(namebuf, ep.achName);

            /* Leave Case of Name Alone -- In Native Form */
            /* (Removed Forced Lowercasing Code) */

            v = PyString_FromString(namebuf);
            if (v == NULL) {
                Py_DECREF(d);
                d = NULL;
                break;
            }
            if (PyList_Append(d, v) != 0) {
                Py_DECREF(v);
                Py_DECREF(d);
                d = NULL;
                break;
            }
            Py_DECREF(v);
        } while (DosFindNext(hdir, &ep, sizeof(ep), &srchcnt) == NO_ERROR && srchcnt > 0);
    }

    return d;
#else

	char *name;
	PyObject *d, *v;
	DIR *dirp;
	struct dirent *ep;
	if (!PyArg_ParseTuple(args, "s:listdir", &name))
		return NULL;
	if ((dirp = opendir(name)) == NULL) {
		return posix_error_with_filename(name);
	}
	if ((d = PyList_New(0)) == NULL) {
		closedir(dirp);
		return NULL;
	}
	while ((ep = readdir(dirp)) != NULL) {
		if (ep->d_name[0] == '.' &&
		    (NAMLEN(ep) == 1 ||
		     (ep->d_name[1] == '.' && NAMLEN(ep) == 2)))
			continue;
		v = PyString_FromStringAndSize(ep->d_name, NAMLEN(ep));
		if (v == NULL) {
			Py_DECREF(d);
			d = NULL;
			break;
		}
		if (PyList_Append(d, v) != 0) {
			Py_DECREF(v);
			Py_DECREF(d);
			d = NULL;
			break;
		}
		Py_DECREF(v);
	}
	closedir(dirp);

	return d;

#endif /* which OS */
}  /* end of posix_listdir */

#ifdef MS_WINDOWS
/* A helper function for abspath on win32 */
static PyObject *
posix__getfullpathname(PyObject *self, PyObject *args)
{
	/* assume encoded strings wont more than double no of chars */
	char inbuf[MAX_PATH*2];
	char *inbufp = inbuf;
	int insize = sizeof(inbuf)/sizeof(inbuf[0]);
	char outbuf[MAX_PATH*2];
	char *temp;
#ifdef Py_WIN_WIDE_FILENAMES
	if (unicode_file_names()) {
		PyUnicodeObject *po;
		if (PyArg_ParseTuple(args, "U|:_getfullpathname", &po)) {
			Py_UNICODE woutbuf[MAX_PATH*2];
			Py_UNICODE *wtemp;
			if (!GetFullPathNameW(PyUnicode_AS_UNICODE(po), 
						sizeof(woutbuf)/sizeof(woutbuf[0]),
						 woutbuf, &wtemp))
				return win32_error("GetFullPathName", "");
			return PyUnicode_FromUnicode(woutbuf, wcslen(woutbuf));
		}
		/* Drop the argument parsing error as narrow strings
		   are also valid. */
		PyErr_Clear();
	}
#endif
	if (!PyArg_ParseTuple (args, "et#:_getfullpathname",
	                       Py_FileSystemDefaultEncoding, &inbufp,
	                       &insize))
		return NULL;
	if (!GetFullPathName(inbuf, sizeof(outbuf)/sizeof(outbuf[0]),
	                     outbuf, &temp))
		return win32_error("GetFullPathName", inbuf);
	return PyString_FromString(outbuf);
} /* end of posix__getfullpathname */
#endif /* MS_WINDOWS */

PyDoc_STRVAR(posix_mkdir__doc__,
"mkdir(path [, mode=0777])\n\n\
Create a directory.");

static PyObject *
posix_mkdir(PyObject *self, PyObject *args)
{
	int res;
	char *path = NULL;
	int mode = 0777;

#ifdef Py_WIN_WIDE_FILENAMES
	if (unicode_file_names()) {
		PyUnicodeObject *po;
		if (PyArg_ParseTuple(args, "U|i:mkdir", &po)) {
			Py_BEGIN_ALLOW_THREADS
			/* PyUnicode_AS_UNICODE OK without thread lock as
			   it is a simple dereference. */
			res = _wmkdir(PyUnicode_AS_UNICODE(po));
			Py_END_ALLOW_THREADS
			if (res < 0)
				return posix_error();
			Py_INCREF(Py_None);
			return Py_None;
		}
		/* Drop the argument parsing error as narrow strings
		   are also valid. */
		PyErr_Clear();
	}
#endif

	if (!PyArg_ParseTuple(args, "et|i:mkdir",
	                      Py_FileSystemDefaultEncoding, &path, &mode))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
#if ( defined(__WATCOMC__) || defined(_MSC_VER) || defined(PYCC_VACPP) ) && !defined(__QNX__)
	res = mkdir(path);
#else
	res = mkdir(path, mode);
#endif
	Py_END_ALLOW_THREADS
	if (res < 0)
		return posix_error_with_allocated_filename(path);
	PyMem_Free(path);
	Py_INCREF(Py_None);
	return Py_None;
}


#ifdef HAVE_NICE
#if defined(HAVE_BROKEN_NICE) && defined(HAVE_SYS_RESOURCE_H)
#if defined(HAVE_GETPRIORITY) && !defined(PRIO_PROCESS)
#include <sys/resource.h>
#endif
#endif

PyDoc_STRVAR(posix_nice__doc__,
"nice(inc) -> new_priority\n\n\
Decrease the priority of process by inc and return the new priority.");

static PyObject *
posix_nice(PyObject *self, PyObject *args)
{
	int increment, value;

	if (!PyArg_ParseTuple(args, "i:nice", &increment))
		return NULL;

	/* There are two flavours of 'nice': one that returns the new
	   priority (as required by almost all standards out there) and the
	   Linux/FreeBSD/BSDI one, which returns '0' on success and advices
	   the use of getpriority() to get the new priority.

	   If we are of the nice family that returns the new priority, we
	   need to clear errno before the call, and check if errno is filled
	   before calling posix_error() on a returnvalue of -1, because the
	   -1 may be the actual new priority! */

	errno = 0;
	value = nice(increment);
#if defined(HAVE_BROKEN_NICE) && defined(HAVE_GETPRIORITY)
	if (value == 0)
		value = getpriority(PRIO_PROCESS, 0);
#endif
	if (value == -1 && errno != 0)
		/* either nice() or getpriority() returned an error */
		return posix_error();
	return PyInt_FromLong((long) value);
}
#endif /* HAVE_NICE */


PyDoc_STRVAR(posix_rename__doc__,
"rename(old, new)\n\n\
Rename a file or directory.");

static PyObject *
posix_rename(PyObject *self, PyObject *args)
{
#ifdef MS_WINDOWS
	return posix_2str(args, "etet:rename", rename, "OO:rename", _wrename);
#else
	return posix_2str(args, "etet:rename", rename, NULL, NULL);
#endif
}


PyDoc_STRVAR(posix_rmdir__doc__,
"rmdir(path)\n\n\
Remove a directory.");

static PyObject *
posix_rmdir(PyObject *self, PyObject *args)
{
#ifdef MS_WINDOWS
	return posix_1str(args, "et:rmdir", rmdir, "U:rmdir", _wrmdir);
#else
	return posix_1str(args, "et:rmdir", rmdir, NULL, NULL);
#endif
}


PyDoc_STRVAR(posix_stat__doc__,
"stat(path) -> stat result\n\n\
Perform a stat system call on the given path.");

static PyObject *
posix_stat(PyObject *self, PyObject *args)
{
#ifdef MS_WINDOWS
	return posix_do_stat(self, args, "et:stat", STAT, "U:stat", _wstati64);
#else
	return posix_do_stat(self, args, "et:stat", STAT, NULL, NULL);
#endif
}


#ifdef HAVE_SYSTEM
PyDoc_STRVAR(posix_system__doc__,
"system(command) -> exit_status\n\n\
Execute the command (a string) in a subshell.");

static PyObject *
posix_system(PyObject *self, PyObject *args)
{
	char *command;
	long sts;
	if (!PyArg_ParseTuple(args, "s:system", &command))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	sts = system(command);
	Py_END_ALLOW_THREADS
	return PyInt_FromLong(sts);
}
#endif


PyDoc_STRVAR(posix_umask__doc__,
"umask(new_mask) -> old_mask\n\n\
Set the current numeric umask and return the previous umask.");

static PyObject *
posix_umask(PyObject *self, PyObject *args)
{
	int i;
	if (!PyArg_ParseTuple(args, "i:umask", &i))
		return NULL;
	i = (int)umask(i);
	if (i < 0)
		return posix_error();
	return PyInt_FromLong((long)i);
}


PyDoc_STRVAR(posix_unlink__doc__,
"unlink(path)\n\n\
Remove a file (same as remove(path)).");

PyDoc_STRVAR(posix_remove__doc__,
"remove(path)\n\n\
Remove a file (same as unlink(path)).");

static PyObject *
posix_unlink(PyObject *self, PyObject *args)
{
#ifdef MS_WINDOWS
	return posix_1str(args, "et:remove", unlink, "U:remove", _wunlink);
#else
	return posix_1str(args, "et:remove", unlink, NULL, NULL);
#endif
}


#ifdef HAVE_UNAME
PyDoc_STRVAR(posix_uname__doc__,
"uname() -> (sysname, nodename, release, version, machine)\n\n\
Return a tuple identifying the current operating system.");

static PyObject *
posix_uname(PyObject *self, PyObject *args)
{
	struct utsname u;
	int res;
	if (!PyArg_ParseTuple(args, ":uname"))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = uname(&u);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return posix_error();
	return Py_BuildValue("(sssss)",
			     u.sysname,
			     u.nodename,
			     u.release,
			     u.version,
			     u.machine);
}
#endif /* HAVE_UNAME */

static int
extract_time(PyObject *t, long* sec, long* usec)
{
	long intval;
	if (PyFloat_Check(t)) {
		double tval = PyFloat_AsDouble(t);
		PyObject *intobj = t->ob_type->tp_as_number->nb_int(t);
		if (!intobj)
			return -1;
		intval = PyInt_AsLong(intobj);
		Py_DECREF(intobj);
		*sec = intval;
		*usec = (long)((tval - intval) * 1e6); /* can't exceed 1000000 */
		if (*usec < 0)
			/* If rounding gave us a negative number,
			   truncate.  */
			*usec = 0;
		return 0;
	}
	intval = PyInt_AsLong(t);
	if (intval == -1 && PyErr_Occurred())
		return -1;
	*sec = intval;
	*usec = 0;
        return 0;
}

PyDoc_STRVAR(posix_utime__doc__,
"utime(path, (atime, utime))\n\
utime(path, None)\n\n\
Set the access and modified time of the file to the given values.  If the\n\
second form is used, set the access and modified times to the current time.");

static PyObject *
posix_utime(PyObject *self, PyObject *args)
{
	char *path;
	long atime, mtime, ausec, musec;
	int res;
	PyObject* arg;

#if defined(HAVE_UTIMES)
	struct timeval buf[2];
#define ATIME buf[0].tv_sec
#define MTIME buf[1].tv_sec
#elif defined(HAVE_UTIME_H)
/* XXX should define struct utimbuf instead, above */
	struct utimbuf buf;
#define ATIME buf.actime
#define MTIME buf.modtime
#define UTIME_ARG &buf
#else /* HAVE_UTIMES */
	time_t buf[2];
#define ATIME buf[0]
#define MTIME buf[1]
#define UTIME_ARG buf
#endif /* HAVE_UTIMES */

	if (!PyArg_ParseTuple(args, "sO:utime", &path, &arg))
		return NULL;
	if (arg == Py_None) {
		/* optional time values not given */
		Py_BEGIN_ALLOW_THREADS
		res = utime(path, NULL);
		Py_END_ALLOW_THREADS
	}
	else if (!PyTuple_Check(arg) || PyTuple_Size(arg) != 2) {
		PyErr_SetString(PyExc_TypeError,
				"utime() arg 2 must be a tuple (atime, mtime)");
		return NULL;
	}
	else {
		if (extract_time(PyTuple_GET_ITEM(arg, 0),
				 &atime, &ausec) == -1)
			return NULL;
		if (extract_time(PyTuple_GET_ITEM(arg, 1),
				 &mtime, &musec) == -1)
			return NULL;
		ATIME = atime;
		MTIME = mtime;
#ifdef HAVE_UTIMES
		buf[0].tv_usec = ausec;
		buf[1].tv_usec = musec;
		Py_BEGIN_ALLOW_THREADS
		res = utimes(path, buf);
		Py_END_ALLOW_THREADS
#else
		Py_BEGIN_ALLOW_THREADS
		res = utime(path, UTIME_ARG);
		Py_END_ALLOW_THREADS
#endif
	}
	if (res < 0)
		return posix_error_with_filename(path);
	Py_INCREF(Py_None);
	return Py_None;
#undef UTIME_ARG
#undef ATIME
#undef MTIME
}


/* Process operations */

PyDoc_STRVAR(posix__exit__doc__,
"_exit(status)\n\n\
Exit to the system with specified status, without normal exit processing.");

static PyObject *
posix__exit(PyObject *self, PyObject *args)
{
	int sts;
	if (!PyArg_ParseTuple(args, "i:_exit", &sts))
		return NULL;
	_exit(sts);
	return NULL; /* Make gcc -Wall happy */
}


#ifdef HAVE_EXECV
PyDoc_STRVAR(posix_execv__doc__,
"execv(path, args)\n\n\
Execute an executable path with arguments, replacing current process.\n\
\n\
	path: path of executable file\n\
	args: tuple or list of strings");

static PyObject *
posix_execv(PyObject *self, PyObject *args)
{
	char *path;
	PyObject *argv;
	char **argvlist;
	int i, argc;
	PyObject *(*getitem)(PyObject *, int);

	/* execv has two arguments: (path, argv), where
	   argv is a list or tuple of strings. */

	if (!PyArg_ParseTuple(args, "sO:execv", &path, &argv))
		return NULL;
	if (PyList_Check(argv)) {
		argc = PyList_Size(argv);
		getitem = PyList_GetItem;
	}
	else if (PyTuple_Check(argv)) {
		argc = PyTuple_Size(argv);
		getitem = PyTuple_GetItem;
	}
	else {
		PyErr_SetString(PyExc_TypeError, "execv() arg 2 must be a tuple or list");
		return NULL;
	}

	if (argc == 0) {
		PyErr_SetString(PyExc_ValueError, "execv() arg 2 must not be empty");
		return NULL;
	}

	argvlist = PyMem_NEW(char *, argc+1);
	if (argvlist == NULL)
		return NULL;
	for (i = 0; i < argc; i++) {
		if (!PyArg_Parse((*getitem)(argv, i), "s", &argvlist[i])) {
			PyMem_DEL(argvlist);
			PyErr_SetString(PyExc_TypeError,
					"execv() arg 2 must contain only strings");
			return NULL;

		}
	}
	argvlist[argc] = NULL;

#ifdef BAD_EXEC_PROTOTYPES
	execv(path, (const char **) argvlist);
#else /* BAD_EXEC_PROTOTYPES */
	execv(path, argvlist);
#endif /* BAD_EXEC_PROTOTYPES */

	/* If we get here it's definitely an error */

	PyMem_DEL(argvlist);
	return posix_error();
}


PyDoc_STRVAR(posix_execve__doc__,
"execve(path, args, env)\n\n\
Execute a path with arguments and environment, replacing current process.\n\
\n\
	path: path of executable file\n\
	args: tuple or list of arguments\n\
	env: dictionary of strings mapping to strings");

static PyObject *
posix_execve(PyObject *self, PyObject *args)
{
	char *path;
	PyObject *argv, *env;
	char **argvlist;
	char **envlist;
	PyObject *key, *val, *keys=NULL, *vals=NULL;
	int i, pos, argc, envc;
	PyObject *(*getitem)(PyObject *, int);

	/* execve has three arguments: (path, argv, env), where
	   argv is a list or tuple of strings and env is a dictionary
	   like posix.environ. */

	if (!PyArg_ParseTuple(args, "sOO:execve", &path, &argv, &env))
		return NULL;
	if (PyList_Check(argv)) {
		argc = PyList_Size(argv);
		getitem = PyList_GetItem;
	}
	else if (PyTuple_Check(argv)) {
		argc = PyTuple_Size(argv);
		getitem = PyTuple_GetItem;
	}
	else {
		PyErr_SetString(PyExc_TypeError, "execve() arg 2 must be a tuple or list");
		return NULL;
	}
	if (!PyMapping_Check(env)) {
		PyErr_SetString(PyExc_TypeError, "execve() arg 3 must be a mapping object");
		return NULL;
	}

	if (argc == 0) {
		PyErr_SetString(PyExc_ValueError,
				"execve() arg 2 must not be empty");
		return NULL;
	}

	argvlist = PyMem_NEW(char *, argc+1);
	if (argvlist == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	for (i = 0; i < argc; i++) {
		if (!PyArg_Parse((*getitem)(argv, i),
				 "s;execve() arg 2 must contain only strings",
				 &argvlist[i]))
		{
			goto fail_1;
		}
	}
	argvlist[argc] = NULL;

	i = PyMapping_Size(env);
	envlist = PyMem_NEW(char *, i + 1);
	if (envlist == NULL) {
		PyErr_NoMemory();
		goto fail_1;
	}
	envc = 0;
	keys = PyMapping_Keys(env);
	vals = PyMapping_Values(env);
	if (!keys || !vals)
		goto fail_2;

	for (pos = 0; pos < i; pos++) {
		char *p, *k, *v;
		size_t len;

		key = PyList_GetItem(keys, pos);
		val = PyList_GetItem(vals, pos);
		if (!key || !val)
			goto fail_2;

		if (!PyArg_Parse(key, "s;execve() arg 3 contains a non-string key", &k) ||
		    !PyArg_Parse(val, "s;execve() arg 3 contains a non-string value", &v))
		{
			goto fail_2;
		}

#if defined(PYOS_OS2)
        /* Omit Pseudo-Env Vars that Would Confuse Programs if Passed On */
        if (stricmp(k, "BEGINLIBPATH") != 0 && stricmp(k, "ENDLIBPATH") != 0) {
#endif
		len = PyString_Size(key) + PyString_Size(val) + 2;
		p = PyMem_NEW(char, len);
		if (p == NULL) {
			PyErr_NoMemory();
			goto fail_2;
		}
		PyOS_snprintf(p, len, "%s=%s", k, v);
		envlist[envc++] = p;
#if defined(PYOS_OS2)
    }
#endif
	}
	envlist[envc] = 0;


#ifdef BAD_EXEC_PROTOTYPES
	execve(path, (const char **)argvlist, envlist);
#else /* BAD_EXEC_PROTOTYPES */
	execve(path, argvlist, envlist);
#endif /* BAD_EXEC_PROTOTYPES */

	/* If we get here it's definitely an error */

	(void) posix_error();

 fail_2:
	while (--envc >= 0)
		PyMem_DEL(envlist[envc]);
	PyMem_DEL(envlist);
 fail_1:
	PyMem_DEL(argvlist);
	Py_XDECREF(vals);
	Py_XDECREF(keys);
	return NULL;
}
#endif /* HAVE_EXECV */


#ifdef HAVE_SPAWNV
PyDoc_STRVAR(posix_spawnv__doc__,
"spawnv(mode, path, args)\n\n\
Execute the program 'path' in a new process.\n\
\n\
	mode: mode of process creation\n\
	path: path of executable file\n\
	args: tuple or list of strings");

static PyObject *
posix_spawnv(PyObject *self, PyObject *args)
{
	char *path;
	PyObject *argv;
	char **argvlist;
	int mode, i, argc;
	Py_intptr_t spawnval;
	PyObject *(*getitem)(PyObject *, int);

	/* spawnv has three arguments: (mode, path, argv), where
	   argv is a list or tuple of strings. */

	if (!PyArg_ParseTuple(args, "isO:spawnv", &mode, &path, &argv))
		return NULL;
	if (PyList_Check(argv)) {
		argc = PyList_Size(argv);
		getitem = PyList_GetItem;
	}
	else if (PyTuple_Check(argv)) {
		argc = PyTuple_Size(argv);
		getitem = PyTuple_GetItem;
	}
	else {
		PyErr_SetString(PyExc_TypeError, "spawnv() arg 2 must be a tuple or list");
		return NULL;
	}

	argvlist = PyMem_NEW(char *, argc+1);
	if (argvlist == NULL)
		return NULL;
	for (i = 0; i < argc; i++) {
		if (!PyArg_Parse((*getitem)(argv, i), "s", &argvlist[i])) {
			PyMem_DEL(argvlist);
			PyErr_SetString(PyExc_TypeError,
					"spawnv() arg 2 must contain only strings");
			return NULL;
		}
	}
	argvlist[argc] = NULL;

#if defined(PYOS_OS2) && defined(PYCC_GCC)
	Py_BEGIN_ALLOW_THREADS
	spawnval = spawnv(mode, path, argvlist);
	Py_END_ALLOW_THREADS
#else
	if (mode == _OLD_P_OVERLAY)
		mode = _P_OVERLAY;

	Py_BEGIN_ALLOW_THREADS
	spawnval = _spawnv(mode, path, argvlist);
	Py_END_ALLOW_THREADS
#endif

	PyMem_DEL(argvlist);

	if (spawnval == -1)
		return posix_error();
	else
#if SIZEOF_LONG == SIZEOF_VOID_P
		return Py_BuildValue("l", (long) spawnval);
#else
		return Py_BuildValue("L", (LONG_LONG) spawnval);
#endif
}


PyDoc_STRVAR(posix_spawnve__doc__,
"spawnve(mode, path, args, env)\n\n\
Execute the program 'path' in a new process.\n\
\n\
	mode: mode of process creation\n\
	path: path of executable file\n\
	args: tuple or list of arguments\n\
	env: dictionary of strings mapping to strings");

static PyObject *
posix_spawnve(PyObject *self, PyObject *args)
{
	char *path;
	PyObject *argv, *env;
	char **argvlist;
	char **envlist;
	PyObject *key, *val, *keys=NULL, *vals=NULL, *res=NULL;
	int mode, i, pos, argc, envc;
	Py_intptr_t spawnval;
	PyObject *(*getitem)(PyObject *, int);

	/* spawnve has four arguments: (mode, path, argv, env), where
	   argv is a list or tuple of strings and env is a dictionary
	   like posix.environ. */

	if (!PyArg_ParseTuple(args, "isOO:spawnve", &mode, &path, &argv, &env))
		return NULL;
	if (PyList_Check(argv)) {
		argc = PyList_Size(argv);
		getitem = PyList_GetItem;
	}
	else if (PyTuple_Check(argv)) {
		argc = PyTuple_Size(argv);
		getitem = PyTuple_GetItem;
	}
	else {
		PyErr_SetString(PyExc_TypeError, "spawnve() arg 2 must be a tuple or list");
		return NULL;
	}
	if (!PyMapping_Check(env)) {
		PyErr_SetString(PyExc_TypeError, "spawnve() arg 3 must be a mapping object");
		return NULL;
	}

	argvlist = PyMem_NEW(char *, argc+1);
	if (argvlist == NULL) {
		PyErr_NoMemory();
		return NULL;
	}
	for (i = 0; i < argc; i++) {
		if (!PyArg_Parse((*getitem)(argv, i),
				 "s;spawnve() arg 2 must contain only strings",
				 &argvlist[i]))
		{
			goto fail_1;
		}
	}
	argvlist[argc] = NULL;

	i = PyMapping_Size(env);
	envlist = PyMem_NEW(char *, i + 1);
	if (envlist == NULL) {
		PyErr_NoMemory();
		goto fail_1;
	}
	envc = 0;
	keys = PyMapping_Keys(env);
	vals = PyMapping_Values(env);
	if (!keys || !vals)
		goto fail_2;

	for (pos = 0; pos < i; pos++) {
		char *p, *k, *v;
		size_t len;

		key = PyList_GetItem(keys, pos);
		val = PyList_GetItem(vals, pos);
		if (!key || !val)
			goto fail_2;

		if (!PyArg_Parse(key, "s;spawnve() arg 3 contains a non-string key", &k) ||
		    !PyArg_Parse(val, "s;spawnve() arg 3 contains a non-string value", &v))
		{
			goto fail_2;
		}
		len = PyString_Size(key) + PyString_Size(val) + 2;
		p = PyMem_NEW(char, len);
		if (p == NULL) {
			PyErr_NoMemory();
			goto fail_2;
		}
		PyOS_snprintf(p, len, "%s=%s", k, v);
		envlist[envc++] = p;
	}
	envlist[envc] = 0;

#if defined(PYOS_OS2) && defined(PYCC_GCC)
	Py_BEGIN_ALLOW_THREADS
	spawnval = spawnve(mode, path, argvlist, envlist);
	Py_END_ALLOW_THREADS
#else
	if (mode == _OLD_P_OVERLAY)
		mode = _P_OVERLAY;

	Py_BEGIN_ALLOW_THREADS
	spawnval = _spawnve(mode, path, argvlist, envlist);
	Py_END_ALLOW_THREADS
#endif

	if (spawnval == -1)
		(void) posix_error();
	else
#if SIZEOF_LONG == SIZEOF_VOID_P
		res = Py_BuildValue("l", (long) spawnval);
#else
		res = Py_BuildValue("L", (LONG_LONG) spawnval);
#endif

 fail_2:
	while (--envc >= 0)
		PyMem_DEL(envlist[envc]);
	PyMem_DEL(envlist);
 fail_1:
	PyMem_DEL(argvlist);
	Py_XDECREF(vals);
	Py_XDECREF(keys);
	return res;
}
#endif /* HAVE_SPAWNV */


#ifdef HAVE_FORK1
PyDoc_STRVAR(posix_fork1__doc__,
"fork1() -> pid\n\n\
Fork a child process with a single multiplexed (i.e., not bound) thread.\n\
\n\
Return 0 to child process and PID of child to parent process.");

static PyObject *
posix_fork1(self, args)
	PyObject *self;
	PyObject *args;
{
	int pid;
	if (!PyArg_ParseTuple(args, ":fork1"))
		return NULL;
	pid = fork1();
	if (pid == -1)
		return posix_error();
	PyOS_AfterFork();
	return PyInt_FromLong((long)pid);
}
#endif


#ifdef HAVE_FORK
PyDoc_STRVAR(posix_fork__doc__,
"fork() -> pid\n\n\
Fork a child process.\n\
Return 0 to child process and PID of child to parent process.");

static PyObject *
posix_fork(PyObject *self, PyObject *args)
{
	int pid;
	if (!PyArg_ParseTuple(args, ":fork"))
		return NULL;
	pid = fork();
	if (pid == -1)
		return posix_error();
	if (pid == 0)
		PyOS_AfterFork();
	return PyInt_FromLong((long)pid);
}
#endif

#if defined(HAVE_OPENPTY) || defined(HAVE_FORKPTY)
#ifdef HAVE_PTY_H
#include <pty.h>
#else
#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif /* HAVE_LIBUTIL_H */
#endif /* HAVE_PTY_H */
#endif /* defined(HAVE_OPENPTY) || defined(HAVE_FORKPTY) */

#if defined(HAVE_OPENPTY) || defined(HAVE__GETPTY)
PyDoc_STRVAR(posix_openpty__doc__,
"openpty() -> (master_fd, slave_fd)\n\n\
Open a pseudo-terminal, returning open fd's for both master and slave end.\n");

static PyObject *
posix_openpty(PyObject *self, PyObject *args)
{
	int master_fd, slave_fd;
#ifndef HAVE_OPENPTY
	char * slave_name;
#endif

	if (!PyArg_ParseTuple(args, ":openpty"))
		return NULL;

#ifdef HAVE_OPENPTY
	if (openpty(&master_fd, &slave_fd, NULL, NULL, NULL) != 0)
		return posix_error();
#else
	slave_name = _getpty(&master_fd, O_RDWR, 0666, 0);
	if (slave_name == NULL)
		return posix_error();

	slave_fd = open(slave_name, O_RDWR);
	if (slave_fd < 0)
		return posix_error();
#endif /* HAVE_OPENPTY */

	return Py_BuildValue("(ii)", master_fd, slave_fd);

}
#endif /* defined(HAVE_OPENPTY) || defined(HAVE__GETPTY) */

#ifdef HAVE_FORKPTY
PyDoc_STRVAR(posix_forkpty__doc__,
"forkpty() -> (pid, master_fd)\n\n\
Fork a new process with a new pseudo-terminal as controlling tty.\n\n\
Like fork(), return 0 as pid to child process, and PID of child to parent.\n\
To both, return fd of newly opened pseudo-terminal.\n");

static PyObject *
posix_forkpty(PyObject *self, PyObject *args)
{
	int master_fd, pid;

	if (!PyArg_ParseTuple(args, ":forkpty"))
		return NULL;
	pid = forkpty(&master_fd, NULL, NULL, NULL);
	if (pid == -1)
		return posix_error();
	if (pid == 0)
		PyOS_AfterFork();
	return Py_BuildValue("(ii)", pid, master_fd);
}
#endif

#ifdef HAVE_GETEGID
PyDoc_STRVAR(posix_getegid__doc__,
"getegid() -> egid\n\n\
Return the current process's effective group id.");

static PyObject *
posix_getegid(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":getegid"))
		return NULL;
	return PyInt_FromLong((long)getegid());
}
#endif


#ifdef HAVE_GETEUID
PyDoc_STRVAR(posix_geteuid__doc__,
"geteuid() -> euid\n\n\
Return the current process's effective user id.");

static PyObject *
posix_geteuid(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":geteuid"))
		return NULL;
	return PyInt_FromLong((long)geteuid());
}
#endif


#ifdef HAVE_GETGID
PyDoc_STRVAR(posix_getgid__doc__,
"getgid() -> gid\n\n\
Return the current process's group id.");

static PyObject *
posix_getgid(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":getgid"))
		return NULL;
	return PyInt_FromLong((long)getgid());
}
#endif


PyDoc_STRVAR(posix_getpid__doc__,
"getpid() -> pid\n\n\
Return the current process id");

static PyObject *
posix_getpid(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":getpid"))
		return NULL;
	return PyInt_FromLong((long)getpid());
}


#ifdef HAVE_GETGROUPS
PyDoc_STRVAR(posix_getgroups__doc__,
"getgroups() -> list of group IDs\n\n\
Return list of supplemental group IDs for the process.");

static PyObject *
posix_getgroups(PyObject *self, PyObject *args)
{
    PyObject *result = NULL;

    if (PyArg_ParseTuple(args, ":getgroups")) {
#ifdef NGROUPS_MAX
#define MAX_GROUPS NGROUPS_MAX
#else
        /* defined to be 16 on Solaris7, so this should be a small number */
#define MAX_GROUPS 64
#endif
        gid_t grouplist[MAX_GROUPS];
        int n;

        n = getgroups(MAX_GROUPS, grouplist);
        if (n < 0)
            posix_error();
        else {
            result = PyList_New(n);
            if (result != NULL) {
                PyObject *o;
                int i;
                for (i = 0; i < n; ++i) {
                    o = PyInt_FromLong((long)grouplist[i]);
                    if (o == NULL) {
                        Py_DECREF(result);
                        result = NULL;
                        break;
                    }
                    PyList_SET_ITEM(result, i, o);
                }
            }
        }
    }
    return result;
}
#endif

#ifdef HAVE_GETPGID
PyDoc_STRVAR(posix_getpgid__doc__,
"getpgid(pid) -> pgid\n\n\
Call the system call getpgid().");

static PyObject *
posix_getpgid(PyObject *self, PyObject *args)
{
	int pid, pgid;
	if (!PyArg_ParseTuple(args, "i:getpgid", &pid))
		return NULL;
	pgid = getpgid(pid);
	if (pgid < 0)
		return posix_error();
	return PyInt_FromLong((long)pgid);
}
#endif /* HAVE_GETPGID */


#ifdef HAVE_GETPGRP
PyDoc_STRVAR(posix_getpgrp__doc__,
"getpgrp() -> pgrp\n\n\
Return the current process group id.");

static PyObject *
posix_getpgrp(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":getpgrp"))
		return NULL;
#ifdef GETPGRP_HAVE_ARG
	return PyInt_FromLong((long)getpgrp(0));
#else /* GETPGRP_HAVE_ARG */
	return PyInt_FromLong((long)getpgrp());
#endif /* GETPGRP_HAVE_ARG */
}
#endif /* HAVE_GETPGRP */


#ifdef HAVE_SETPGRP
PyDoc_STRVAR(posix_setpgrp__doc__,
"setpgrp()\n\n\
Make this process a session leader.");

static PyObject *
posix_setpgrp(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":setpgrp"))
		return NULL;
#ifdef SETPGRP_HAVE_ARG
	if (setpgrp(0, 0) < 0)
#else /* SETPGRP_HAVE_ARG */
	if (setpgrp() < 0)
#endif /* SETPGRP_HAVE_ARG */
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}

#endif /* HAVE_SETPGRP */

#ifdef HAVE_GETPPID
PyDoc_STRVAR(posix_getppid__doc__,
"getppid() -> ppid\n\n\
Return the parent's process id.");

static PyObject *
posix_getppid(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":getppid"))
		return NULL;
	return PyInt_FromLong((long)getppid());
}
#endif


#ifdef HAVE_GETLOGIN
PyDoc_STRVAR(posix_getlogin__doc__,
"getlogin() -> string\n\n\
Return the actual login name.");

static PyObject *
posix_getlogin(PyObject *self, PyObject *args)
{
    PyObject *result = NULL;

    if (PyArg_ParseTuple(args, ":getlogin")) {
        char *name;
        int old_errno = errno;

        errno = 0;
        name = getlogin();
        if (name == NULL) {
            if (errno)
                posix_error();
            else
                PyErr_SetString(PyExc_OSError,
                                "unable to determine login name");
        }
        else
            result = PyString_FromString(name);
        errno = old_errno;
    }
    return result;
}
#endif

#ifdef HAVE_GETUID
PyDoc_STRVAR(posix_getuid__doc__,
"getuid() -> uid\n\n\
Return the current process's user id.");

static PyObject *
posix_getuid(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":getuid"))
		return NULL;
	return PyInt_FromLong((long)getuid());
}
#endif


#ifdef HAVE_KILL
PyDoc_STRVAR(posix_kill__doc__,
"kill(pid, sig)\n\n\
Kill a process with a signal.");

static PyObject *
posix_kill(PyObject *self, PyObject *args)
{
	int pid, sig;
	if (!PyArg_ParseTuple(args, "ii:kill", &pid, &sig))
		return NULL;
#if defined(PYOS_OS2) && !defined(PYCC_GCC)
    if (sig == XCPT_SIGNAL_INTR || sig == XCPT_SIGNAL_BREAK) {
        APIRET rc;
        if ((rc = DosSendSignalException(pid, sig)) != NO_ERROR)
            return os2_error(rc);

    } else if (sig == XCPT_SIGNAL_KILLPROC) {
        APIRET rc;
        if ((rc = DosKillProcess(DKP_PROCESS, pid)) != NO_ERROR)
            return os2_error(rc);

    } else
        return NULL; /* Unrecognized Signal Requested */
#else
	if (kill(pid, sig) == -1)
		return posix_error();
#endif
	Py_INCREF(Py_None);
	return Py_None;
}
#endif

#ifdef HAVE_KILLPG
PyDoc_STRVAR(posix_killpg__doc__,
"killpg(pgid, sig)\n\n\
Kill a process group with a signal.");

static PyObject *
posix_killpg(PyObject *self, PyObject *args)
{
	int pgid, sig;
	if (!PyArg_ParseTuple(args, "ii:killpg", &pgid, &sig))
		return NULL;
	if (killpg(pgid, sig) == -1)
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif

#ifdef HAVE_PLOCK

#ifdef HAVE_SYS_LOCK_H
#include <sys/lock.h>
#endif

PyDoc_STRVAR(posix_plock__doc__,
"plock(op)\n\n\
Lock program segments into memory.");

static PyObject *
posix_plock(PyObject *self, PyObject *args)
{
	int op;
	if (!PyArg_ParseTuple(args, "i:plock", &op))
		return NULL;
	if (plock(op) == -1)
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif


#ifdef HAVE_POPEN
PyDoc_STRVAR(posix_popen__doc__,
"popen(command [, mode='r' [, bufsize]]) -> pipe\n\n\
Open a pipe to/from a command returning a file object.");

#if defined(PYOS_OS2)
#if defined(PYCC_VACPP)
static int
async_system(const char *command)
{
    char        *p, errormsg[256], args[1024];
    RESULTCODES  rcodes;
    APIRET       rc;
    char        *shell = getenv("COMSPEC");
    if (!shell)
        shell = "cmd";

    strcpy(args, shell);
    p = &args[ strlen(args)+1 ];
    strcpy(p, "/c ");
    strcat(p, command);
    p += strlen(p) + 1;
    *p = '\0';

    rc = DosExecPgm(errormsg, sizeof(errormsg),
                    EXEC_ASYNC, /* Execute Async w/o Wait for Results */
                    args,
                    NULL,       /* Inherit Parent's Environment */
                    &rcodes, shell);
    return rc;
}

static FILE *
popen(const char *command, const char *mode, int pipesize, int *err)
{
    HFILE    rhan, whan;
    FILE    *retfd = NULL;
    APIRET   rc = DosCreatePipe(&rhan, &whan, pipesize);

    if (rc != NO_ERROR) {
	*err = rc;
        return NULL; /* ERROR - Unable to Create Anon Pipe */
    }

    if (strchr(mode, 'r') != NULL) { /* Treat Command as a Data Source */
        int oldfd = dup(1);      /* Save STDOUT Handle in Another Handle */

        DosEnterCritSec();      /* Stop Other Threads While Changing Handles */
        close(1);                /* Make STDOUT Available for Reallocation */

        if (dup2(whan, 1) == 0) {      /* Connect STDOUT to Pipe Write Side */
            DosClose(whan);            /* Close Now-Unused Pipe Write Handle */

            rc = async_system(command);
        }

        dup2(oldfd, 1);          /* Reconnect STDOUT to Original Handle */
        DosExitCritSec();        /* Now Allow Other Threads to Run */

        if (rc == NO_ERROR)
            retfd = fdopen(rhan, mode); /* And Return Pipe Read Handle */

        close(oldfd);            /* And Close Saved STDOUT Handle */
        return retfd;            /* Return fd of Pipe or NULL if Error */

    } else if (strchr(mode, 'w')) { /* Treat Command as a Data Sink */
        int oldfd = dup(0);      /* Save STDIN Handle in Another Handle */

        DosEnterCritSec();      /* Stop Other Threads While Changing Handles */
        close(0);                /* Make STDIN Available for Reallocation */

        if (dup2(rhan, 0) == 0)     { /* Connect STDIN to Pipe Read Side */
            DosClose(rhan);           /* Close Now-Unused Pipe Read Handle */

            rc = async_system(command);
        }

        dup2(oldfd, 0);          /* Reconnect STDIN to Original Handle */
        DosExitCritSec();        /* Now Allow Other Threads to Run */

        if (rc == NO_ERROR)
            retfd = fdopen(whan, mode); /* And Return Pipe Write Handle */

        close(oldfd);            /* And Close Saved STDIN Handle */
        return retfd;            /* Return fd of Pipe or NULL if Error */

    } else {
	*err = ERROR_INVALID_ACCESS;
        return NULL; /* ERROR - Invalid Mode (Neither Read nor Write) */
    }
}

static PyObject *
posix_popen(PyObject *self, PyObject *args)
{
	char *name;
	char *mode = "r";
	int   err, bufsize = -1;
	FILE *fp;
	PyObject *f;
	if (!PyArg_ParseTuple(args, "s|si:popen", &name, &mode, &bufsize))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	fp = popen(name, mode, (bufsize > 0) ? bufsize : 4096, &err);
	Py_END_ALLOW_THREADS
	if (fp == NULL)
		return os2_error(err);

	f = PyFile_FromFile(fp, name, mode, fclose);
	if (f != NULL)
		PyFile_SetBufSize(f, bufsize);
	return f;
}

#elif defined(PYCC_GCC)

/* standard posix version of popen() support */
static PyObject *
posix_popen(PyObject *self, PyObject *args)
{
	char *name;
	char *mode = "r";
	int bufsize = -1;
	FILE *fp;
	PyObject *f;
	if (!PyArg_ParseTuple(args, "s|si:popen", &name, &mode, &bufsize))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	fp = popen(name, mode);
	Py_END_ALLOW_THREADS
	if (fp == NULL)
		return posix_error();
	f = PyFile_FromFile(fp, name, mode, pclose);
	if (f != NULL)
		PyFile_SetBufSize(f, bufsize);
	return f;
}

/* fork() under OS/2 has lots'o'warts
 * EMX supports pipe() and spawn*() so we can synthesize popen[234]()
 * most of this code is a ripoff of the win32 code, but using the
 * capabilities of EMX's C library routines
 */

/* These tell _PyPopen() whether to return 1, 2, or 3 file objects. */
#define POPEN_1 1
#define POPEN_2 2
#define POPEN_3 3
#define POPEN_4 4

static PyObject *_PyPopen(char *, int, int, int);
static int _PyPclose(FILE *file);

/*
 * Internal dictionary mapping popen* file pointers to process handles,
 * for use when retrieving the process exit code.  See _PyPclose() below
 * for more information on this dictionary's use.
 */
static PyObject *_PyPopenProcs = NULL;

/* os2emx version of popen2()
 *
 * The result of this function is a pipe (file) connected to the
 * process's stdin, and a pipe connected to the process's stdout.
 */

static PyObject *
os2emx_popen2(PyObject *self, PyObject  *args)
{
	PyObject *f;
	int tm=0;

	char *cmdstring;
	char *mode = "t";
	int bufsize = -1;
	if (!PyArg_ParseTuple(args, "s|si:popen2", &cmdstring, &mode, &bufsize))
		return NULL;

	if (*mode == 't')
		tm = O_TEXT;
	else if (*mode != 'b') {
		PyErr_SetString(PyExc_ValueError, "mode must be 't' or 'b'");
		return NULL;
	} else
		tm = O_BINARY;

	f = _PyPopen(cmdstring, tm, POPEN_2, bufsize);

	return f;
}

/*
 * Variation on os2emx.popen2
 *
 * The result of this function is 3 pipes - the process's stdin,
 * stdout and stderr
 */

static PyObject *
os2emx_popen3(PyObject *self, PyObject *args)
{
	PyObject *f;
	int tm = 0;

	char *cmdstring;
	char *mode = "t";
	int bufsize = -1;
	if (!PyArg_ParseTuple(args, "s|si:popen3", &cmdstring, &mode, &bufsize))
		return NULL;

	if (*mode == 't')
		tm = O_TEXT;
	else if (*mode != 'b') {
		PyErr_SetString(PyExc_ValueError, "mode must be 't' or 'b'");
		return NULL;
	} else
		tm = O_BINARY;

	f = _PyPopen(cmdstring, tm, POPEN_3, bufsize);

	return f;
}

/*
 * Variation on os2emx.popen2
 *
 * The result of this function is 2 pipes - the processes stdin, 
 * and stdout+stderr combined as a single pipe.
 */

static PyObject *
os2emx_popen4(PyObject *self, PyObject  *args)
{
	PyObject *f;
	int tm = 0;

	char *cmdstring;
	char *mode = "t";
	int bufsize = -1;
	if (!PyArg_ParseTuple(args, "s|si:popen4", &cmdstring, &mode, &bufsize))
		return NULL;

	if (*mode == 't')
		tm = O_TEXT;
	else if (*mode != 'b') {
		PyErr_SetString(PyExc_ValueError, "mode must be 't' or 'b'");
		return NULL;
	} else
		tm = O_BINARY;

	f = _PyPopen(cmdstring, tm, POPEN_4, bufsize);

	return f;
}

/* a couple of structures for convenient handling of multiple
 * file handles and pipes
 */
struct file_ref
{
	int handle;
	int flags;
};

struct pipe_ref
{
	int rd;
	int wr;
};

/* The following code is derived from the win32 code */

static PyObject *
_PyPopen(char *cmdstring, int mode, int n, int bufsize)
{
	struct file_ref stdio[3];
	struct pipe_ref p_fd[3];
	FILE *p_s[3];
	int file_count, i, pipe_err, pipe_pid;
	char *shell, *sh_name, *opt, *rd_mode, *wr_mode;
	PyObject *f, *p_f[3];

	/* file modes for subsequent fdopen's on pipe handles */
	if (mode == O_TEXT)
	{
		rd_mode = "rt";
		wr_mode = "wt";
	}
	else
	{
		rd_mode = "rb";
		wr_mode = "wb";
	}

	/* prepare shell references */
	if ((shell = getenv("EMXSHELL")) == NULL)
		if ((shell = getenv("COMSPEC")) == NULL)
		{
			errno = ENOENT;
			return posix_error();
		}

	sh_name = _getname(shell);
	if (stricmp(sh_name, "cmd.exe") == 0 || stricmp(sh_name, "4os2.exe") == 0)
		opt = "/c";
	else
		opt = "-c";

	/* save current stdio fds + their flags, and set not inheritable */
	i = pipe_err = 0;
	while (pipe_err >= 0 && i < 3)
	{
		pipe_err = stdio[i].handle = dup(i);
		stdio[i].flags = fcntl(i, F_GETFD, 0);
		fcntl(stdio[i].handle, F_SETFD, stdio[i].flags | FD_CLOEXEC);
		i++;
	}
	if (pipe_err < 0)
	{
		/* didn't get them all saved - clean up and bail out */
		int saved_err = errno;
		while (i-- > 0)
		{
			close(stdio[i].handle);
		}
		errno = saved_err;
		return posix_error();
	}

	/* create pipe ends */
	file_count = 2;
	if (n == POPEN_3)
		file_count = 3;
	i = pipe_err = 0;
	while ((pipe_err == 0) && (i < file_count))
		pipe_err = pipe((int *)&p_fd[i++]);
	if (pipe_err < 0)
	{
		/* didn't get them all made - clean up and bail out */
		while (i-- > 0)
		{
			close(p_fd[i].wr);
			close(p_fd[i].rd);
		}
		errno = EPIPE;
		return posix_error();
	}

	/* change the actual standard IO streams over temporarily,
	 * making the retained pipe ends non-inheritable
	 */
	pipe_err = 0;

	/* - stdin */
	if (dup2(p_fd[0].rd, 0) == 0)
	{
		close(p_fd[0].rd);
		i = fcntl(p_fd[0].wr, F_GETFD, 0);
		fcntl(p_fd[0].wr, F_SETFD, i | FD_CLOEXEC);
		if ((p_s[0] = fdopen(p_fd[0].wr, wr_mode)) == NULL)
		{
			close(p_fd[0].wr);
			pipe_err = -1;
		}
	}
	else
	{
		pipe_err = -1;
	}

	/* - stdout */
	if (pipe_err == 0)
	{
		if (dup2(p_fd[1].wr, 1) == 1)
		{
			close(p_fd[1].wr);
			i = fcntl(p_fd[1].rd, F_GETFD, 0);
			fcntl(p_fd[1].rd, F_SETFD, i | FD_CLOEXEC);
			if ((p_s[1] = fdopen(p_fd[1].rd, rd_mode)) == NULL)
			{
				close(p_fd[1].rd);
				pipe_err = -1;
			}
		}
		else
		{
			pipe_err = -1;
		}
	}

	/* - stderr, as required */
	if (pipe_err == 0)
		switch (n)
		{
			case POPEN_3:
			{
				if (dup2(p_fd[2].wr, 2) == 2)
				{
					close(p_fd[2].wr);
					i = fcntl(p_fd[2].rd, F_GETFD, 0);
					fcntl(p_fd[2].rd, F_SETFD, i | FD_CLOEXEC);
					if ((p_s[2] = fdopen(p_fd[2].rd, rd_mode)) == NULL)
					{
						close(p_fd[2].rd);
						pipe_err = -1;
					}
				}
				else
				{
					pipe_err = -1;
				}
				break;
			}

			case POPEN_4:
			{
				if (dup2(1, 2) != 2)
				{
					pipe_err = -1;
				}
				break;
			}
		}

	/* spawn the child process */
	if (pipe_err == 0)
	{
		pipe_pid = spawnlp(P_NOWAIT, shell, shell, opt, cmdstring, (char *)0);
		if (pipe_pid == -1)
		{
			pipe_err = -1;
		}
		else
		{
			/* save the PID into the FILE structure
			 * NOTE: this implementation doesn't actually
			 * take advantage of this, but do it for
			 * completeness - AIM Apr01
			 */
			for (i = 0; i < file_count; i++)
				p_s[i]->_pid = pipe_pid;
		}
	}

	/* reset standard IO to normal */
	for (i = 0; i < 3; i++)
	{
		dup2(stdio[i].handle, i);
		fcntl(i, F_SETFD, stdio[i].flags);
		close(stdio[i].handle);
	}

	/* if any remnant problems, clean up and bail out */
	if (pipe_err < 0)
	{
		for (i = 0; i < 3; i++)
		{
			close(p_fd[i].rd);
			close(p_fd[i].wr);
		}
		errno = EPIPE;
		return posix_error_with_filename(cmdstring);
	}

	/* build tuple of file objects to return */
	if ((p_f[0] = PyFile_FromFile(p_s[0], cmdstring, wr_mode, _PyPclose)) != NULL)
		PyFile_SetBufSize(p_f[0], bufsize);
	if ((p_f[1] = PyFile_FromFile(p_s[1], cmdstring, rd_mode, _PyPclose)) != NULL)
		PyFile_SetBufSize(p_f[1], bufsize);
	if (n == POPEN_3)
	{
		if ((p_f[2] = PyFile_FromFile(p_s[2], cmdstring, rd_mode, _PyPclose)) != NULL)
			PyFile_SetBufSize(p_f[0], bufsize);
		f = Py_BuildValue("OOO", p_f[0], p_f[1], p_f[2]);
	}
	else
		f = Py_BuildValue("OO", p_f[0], p_f[1]);

	/*
	 * Insert the files we've created into the process dictionary
	 * all referencing the list with the process handle and the
	 * initial number of files (see description below in _PyPclose).
	 * Since if _PyPclose later tried to wait on a process when all
	 * handles weren't closed, it could create a deadlock with the
	 * child, we spend some energy here to try to ensure that we
	 * either insert all file handles into the dictionary or none
	 * at all.  It's a little clumsy with the various popen modes
	 * and variable number of files involved.
	 */
	if (!_PyPopenProcs)
	{
		_PyPopenProcs = PyDict_New();
	}

	if (_PyPopenProcs)
	{
		PyObject *procObj, *pidObj, *intObj, *fileObj[3];
		int ins_rc[3];

		fileObj[0] = fileObj[1] = fileObj[2] = NULL;
		ins_rc[0]  = ins_rc[1]  = ins_rc[2]  = 0;

		procObj = PyList_New(2);
		pidObj = PyInt_FromLong((long) pipe_pid);
		intObj = PyInt_FromLong((long) file_count);

		if (procObj && pidObj && intObj)
		{
			PyList_SetItem(procObj, 0, pidObj);
			PyList_SetItem(procObj, 1, intObj);

			fileObj[0] = PyLong_FromVoidPtr(p_s[0]);
			if (fileObj[0])
			{
			    ins_rc[0] = PyDict_SetItem(_PyPopenProcs,
						       fileObj[0],
						       procObj);
			}
			fileObj[1] = PyLong_FromVoidPtr(p_s[1]);
			if (fileObj[1])
			{
			    ins_rc[1] = PyDict_SetItem(_PyPopenProcs,
						       fileObj[1],
						       procObj);
			}
			if (file_count >= 3)
			{
				fileObj[2] = PyLong_FromVoidPtr(p_s[2]);
				if (fileObj[2])
				{
				    ins_rc[2] = PyDict_SetItem(_PyPopenProcs,
							       fileObj[2],
							       procObj);
				}
			}

			if (ins_rc[0] < 0 || !fileObj[0] ||
			    ins_rc[1] < 0 || (file_count > 1 && !fileObj[1]) ||
			    ins_rc[2] < 0 || (file_count > 2 && !fileObj[2]))
			{
				/* Something failed - remove any dictionary
				 * entries that did make it.
				 */
				if (!ins_rc[0] && fileObj[0])
				{
					PyDict_DelItem(_PyPopenProcs,
							fileObj[0]);
				}
				if (!ins_rc[1] && fileObj[1])
				{
					PyDict_DelItem(_PyPopenProcs,
							fileObj[1]);
				}
				if (!ins_rc[2] && fileObj[2])
				{
					PyDict_DelItem(_PyPopenProcs,
							fileObj[2]);
				}
			}
		}
		     
		/*
		 * Clean up our localized references for the dictionary keys
		 * and value since PyDict_SetItem will Py_INCREF any copies
		 * that got placed in the dictionary.
		 */
		Py_XDECREF(procObj);
		Py_XDECREF(fileObj[0]);
		Py_XDECREF(fileObj[1]);
		Py_XDECREF(fileObj[2]);
	}

	/* Child is launched. */
	return f;
}

/*
 * Wrapper for fclose() to use for popen* files, so we can retrieve the
 * exit code for the child process and return as a result of the close.
 *
 * This function uses the _PyPopenProcs dictionary in order to map the
 * input file pointer to information about the process that was
 * originally created by the popen* call that created the file pointer.
 * The dictionary uses the file pointer as a key (with one entry
 * inserted for each file returned by the original popen* call) and a
 * single list object as the value for all files from a single call.
 * The list object contains the Win32 process handle at [0], and a file
 * count at [1], which is initialized to the total number of file
 * handles using that list.
 *
 * This function closes whichever handle it is passed, and decrements
 * the file count in the dictionary for the process handle pointed to
 * by this file.  On the last close (when the file count reaches zero),
 * this function will wait for the child process and then return its
 * exit code as the result of the close() operation.  This permits the
 * files to be closed in any order - it is always the close() of the
 * final handle that will return the exit code.
 */

 /* RED_FLAG 31-Aug-2000 Tim
  * This is always called (today!) between a pair of
  * Py_BEGIN_ALLOW_THREADS/ Py_END_ALLOW_THREADS
  * macros.  So the thread running this has no valid thread state, as
  * far as Python is concerned.  However, this calls some Python API
  * functions that cannot be called safely without a valid thread
  * state, in particular PyDict_GetItem.
  * As a temporary hack (although it may last for years ...), we
  * *rely* on not having a valid thread state in this function, in
  * order to create our own "from scratch".
  * This will deadlock if _PyPclose is ever called by a thread
  * holding the global lock.
  * (The OS/2 EMX thread support appears to cover the case where the
  *  lock is already held - AIM Apr01)
  */

static int _PyPclose(FILE *file)
{
	int result;
	int exit_code;
	int pipe_pid;
	PyObject *procObj, *pidObj, *intObj, *fileObj;
	int file_count;
#ifdef WITH_THREAD
	PyInterpreterState* pInterpreterState;
	PyThreadState* pThreadState;
#endif

	/* Close the file handle first, to ensure it can't block the
	 * child from exiting if it's the last handle.
	 */
	result = fclose(file);

#ifdef WITH_THREAD
	/* Bootstrap a valid thread state into existence. */
	pInterpreterState = PyInterpreterState_New();
	if (!pInterpreterState) {
		/* Well, we're hosed now!  We don't have a thread
		 * state, so can't call a nice error routine, or raise
		 * an exception.  Just die.
		 */
		 Py_FatalError("unable to allocate interpreter state "
		 	       "when closing popen object.");
		 return -1;  /* unreachable */
	}
	pThreadState = PyThreadState_New(pInterpreterState);
	if (!pThreadState) {
		 Py_FatalError("unable to allocate thread state "
		 	       "when closing popen object.");
		 return -1;  /* unreachable */
	}
	/* Grab the global lock.  Note that this will deadlock if the
	 * current thread already has the lock! (see RED_FLAG comments
	 * before this function)
	 */
	PyEval_RestoreThread(pThreadState);
#endif

	if (_PyPopenProcs)
	{
		if ((fileObj = PyLong_FromVoidPtr(file)) != NULL &&
		    (procObj = PyDict_GetItem(_PyPopenProcs,
					      fileObj)) != NULL &&
		    (pidObj = PyList_GetItem(procObj,0)) != NULL &&
		    (intObj = PyList_GetItem(procObj,1)) != NULL)
		{
			pipe_pid = (int) PyInt_AsLong(pidObj);
			file_count = (int) PyInt_AsLong(intObj);

			if (file_count > 1)
			{
				/* Still other files referencing process */
				file_count--;
				PyList_SetItem(procObj,1,
					       PyInt_FromLong((long) file_count));
			}
			else
			{
				/* Last file for this process */
				if (result != EOF &&
				    waitpid(pipe_pid, &exit_code, 0) == pipe_pid)
				{
					/* extract exit status */
					if (WIFEXITED(exit_code))
					{
						result = WEXITSTATUS(exit_code);
					}
					else
					{
						errno = EPIPE;
						result = -1;
					}
				}
				else
				{
					/* Indicate failure - this will cause the file object
					 * to raise an I/O error and translate the last
					 * error code from errno.  We do have a problem with
					 * last errors that overlap the normal errno table,
					 * but that's a consistent problem with the file object.
					 */
					result = -1;
				}
			}

			/* Remove this file pointer from dictionary */
			PyDict_DelItem(_PyPopenProcs, fileObj);

			if (PyDict_Size(_PyPopenProcs) == 0)
			{
				Py_DECREF(_PyPopenProcs);
				_PyPopenProcs = NULL;
			}

		} /* if object retrieval ok */

		Py_XDECREF(fileObj);
	} /* if _PyPopenProcs */

#ifdef WITH_THREAD
	/* Tear down the thread & interpreter states.
	 * Note that interpreter state clear & delete functions automatically
	 * call the thread clear & delete functions, and indeed insist on
	 * doing that themselves.  The lock must be held during the clear, but
	 * need not be held during the delete.
	 */
	PyInterpreterState_Clear(pInterpreterState);
	PyEval_ReleaseThread(pThreadState);
	PyInterpreterState_Delete(pInterpreterState);
#endif

	return result;
}

#endif /* PYCC_??? */

#elif defined(MS_WINDOWS)

/*
 * Portable 'popen' replacement for Win32.
 *
 * Written by Bill Tutt <billtut@microsoft.com>.  Minor tweaks
 * and 2.0 integration by Fredrik Lundh <fredrik@pythonware.com>
 * Return code handling by David Bolen <db3l@fitlinxx.com>.
 */

#include <malloc.h>
#include <io.h>
#include <fcntl.h>

/* These tell _PyPopen() wether to return 1, 2, or 3 file objects. */
#define POPEN_1 1
#define POPEN_2 2
#define POPEN_3 3
#define POPEN_4 4

static PyObject *_PyPopen(char *, int, int);
static int _PyPclose(FILE *file);

/*
 * Internal dictionary mapping popen* file pointers to process handles,
 * for use when retrieving the process exit code.  See _PyPclose() below
 * for more information on this dictionary's use.
 */
static PyObject *_PyPopenProcs = NULL;


/* popen that works from a GUI.
 *
 * The result of this function is a pipe (file) connected to the
 * processes stdin or stdout, depending on the requested mode.
 */

static PyObject *
posix_popen(PyObject *self, PyObject *args)
{
	PyObject *f, *s;
	int tm = 0;

	char *cmdstring;
	char *mode = "r";
	int bufsize = -1;
	if (!PyArg_ParseTuple(args, "s|si:popen", &cmdstring, &mode, &bufsize))
		return NULL;

	s = PyTuple_New(0);

	if (*mode == 'r')
		tm = _O_RDONLY;
	else if (*mode != 'w') {
		PyErr_SetString(PyExc_ValueError, "popen() arg 2 must be 'r' or 'w'");
		return NULL;
	} else
		tm = _O_WRONLY;

	if (bufsize != -1) {
		PyErr_SetString(PyExc_ValueError, "popen() arg 3 must be -1");
		return NULL;
	}

	if (*(mode+1) == 't')
		f = _PyPopen(cmdstring, tm | _O_TEXT, POPEN_1);
	else if (*(mode+1) == 'b')
		f = _PyPopen(cmdstring, tm | _O_BINARY, POPEN_1);
	else
		f = _PyPopen(cmdstring, tm | _O_TEXT, POPEN_1);

	return f;
}

/* Variation on win32pipe.popen
 *
 * The result of this function is a pipe (file) connected to the
 * process's stdin, and a pipe connected to the process's stdout.
 */

static PyObject *
win32_popen2(PyObject *self, PyObject  *args)
{
	PyObject *f;
	int tm=0;

	char *cmdstring;
	char *mode = "t";
	int bufsize = -1;
	if (!PyArg_ParseTuple(args, "s|si:popen2", &cmdstring, &mode, &bufsize))
		return NULL;

	if (*mode == 't')
		tm = _O_TEXT;
	else if (*mode != 'b') {
		PyErr_SetString(PyExc_ValueError, "popen2() arg 2 must be 't' or 'b'");
		return NULL;
	} else
		tm = _O_BINARY;

	if (bufsize != -1) {
		PyErr_SetString(PyExc_ValueError, "popen2() arg 3 must be -1");
		return NULL;
	}

	f = _PyPopen(cmdstring, tm, POPEN_2);

	return f;
}

/*
 * Variation on <om win32pipe.popen>
 *
 * The result of this function is 3 pipes - the process's stdin,
 * stdout and stderr
 */

static PyObject *
win32_popen3(PyObject *self, PyObject *args)
{
	PyObject *f;
	int tm = 0;

	char *cmdstring;
	char *mode = "t";
	int bufsize = -1;
	if (!PyArg_ParseTuple(args, "s|si:popen3", &cmdstring, &mode, &bufsize))
		return NULL;

	if (*mode == 't')
		tm = _O_TEXT;
	else if (*mode != 'b') {
		PyErr_SetString(PyExc_ValueError, "popen3() arg 2 must be 't' or 'b'");
		return NULL;
	} else
		tm = _O_BINARY;

	if (bufsize != -1) {
		PyErr_SetString(PyExc_ValueError, "popen3() arg 3 must be -1");
		return NULL;
	}

	f = _PyPopen(cmdstring, tm, POPEN_3);

	return f;
}

/*
 * Variation on win32pipe.popen
 *
 * The result of this function is 2 pipes - the processes stdin,
 * and stdout+stderr combined as a single pipe.
 */

static PyObject *
win32_popen4(PyObject *self, PyObject  *args)
{
	PyObject *f;
	int tm = 0;

	char *cmdstring;
	char *mode = "t";
	int bufsize = -1;
	if (!PyArg_ParseTuple(args, "s|si:popen4", &cmdstring, &mode, &bufsize))
		return NULL;

	if (*mode == 't')
		tm = _O_TEXT;
	else if (*mode != 'b') {
		PyErr_SetString(PyExc_ValueError, "popen4() arg 2 must be 't' or 'b'");
		return NULL;
	} else
		tm = _O_BINARY;

	if (bufsize != -1) {
		PyErr_SetString(PyExc_ValueError, "popen4() arg 3 must be -1");
		return NULL;
	}

	f = _PyPopen(cmdstring, tm, POPEN_4);

	return f;
}

static BOOL
_PyPopenCreateProcess(char *cmdstring,
		      HANDLE hStdin,
		      HANDLE hStdout,
		      HANDLE hStderr,
		      HANDLE *hProcess)
{
	PROCESS_INFORMATION piProcInfo;
	STARTUPINFO siStartInfo;
	DWORD dwProcessFlags = 0;  /* no NEW_CONSOLE by default for Ctrl+C handling */
	char *s1,*s2, *s3 = " /c ";
	const char *szConsoleSpawn = "w9xpopen.exe";
	int i;
	int x;

	if (i = GetEnvironmentVariable("COMSPEC",NULL,0)) {
		char *comshell;

		s1 = (char *)alloca(i);
		if (!(x = GetEnvironmentVariable("COMSPEC", s1, i)))
			return x;

		/* Explicitly check if we are using COMMAND.COM.  If we are
		 * then use the w9xpopen hack.
		 */
		comshell = s1 + x;
		while (comshell >= s1 && *comshell != '\\')
			--comshell;
		++comshell;

		if (GetVersion() < 0x80000000 &&
		    _stricmp(comshell, "command.com") != 0) {
			/* NT/2000 and not using command.com. */
			x = i + strlen(s3) + strlen(cmdstring) + 1;
			s2 = (char *)alloca(x);
			ZeroMemory(s2, x);
			PyOS_snprintf(s2, x, "%s%s%s", s1, s3, cmdstring);
		}
		else {
			/*
			 * Oh gag, we're on Win9x or using COMMAND.COM. Use
			 * the workaround listed in KB: Q150956
			 */
			char modulepath[_MAX_PATH];
			struct stat statinfo;
			GetModuleFileName(NULL, modulepath, sizeof(modulepath));
			for (i = x = 0; modulepath[i]; i++)
				if (modulepath[i] == SEP)
					x = i+1;
			modulepath[x] = '\0';
			/* Create the full-name to w9xpopen, so we can test it exists */
			strncat(modulepath,
			        szConsoleSpawn,
			        (sizeof(modulepath)/sizeof(modulepath[0]))
			               -strlen(modulepath));
			if (stat(modulepath, &statinfo) != 0) {
				/* Eeek - file-not-found - possibly an embedding
				   situation - see if we can locate it in sys.prefix
				*/
				strncpy(modulepath,
				        Py_GetExecPrefix(),
				        sizeof(modulepath)/sizeof(modulepath[0]));
				if (modulepath[strlen(modulepath)-1] != '\\')
					strcat(modulepath, "\\");
				strncat(modulepath,
				        szConsoleSpawn,
				        (sizeof(modulepath)/sizeof(modulepath[0]))
				               -strlen(modulepath));
				/* No where else to look - raise an easily identifiable
				   error, rather than leaving Windows to report
				   "file not found" - as the user is probably blissfully
				   unaware this shim EXE is used, and it will confuse them.
				   (well, it confused me for a while ;-)
				*/
				if (stat(modulepath, &statinfo) != 0) {
					PyErr_Format(PyExc_RuntimeError,
					    "Can not locate '%s' which is needed "
					    "for popen to work with your shell "
					    "or platform.",
					    szConsoleSpawn);
					return FALSE;
				}
			}
			x = i + strlen(s3) + strlen(cmdstring) + 1 +
				strlen(modulepath) +
				strlen(szConsoleSpawn) + 1;

			s2 = (char *)alloca(x);
			ZeroMemory(s2, x);
			/* To maintain correct argument passing semantics,
			   we pass the command-line as it stands, and allow
			   quoting to be applied.  w9xpopen.exe will then
			   use its argv vector, and re-quote the necessary
			   args for the ultimate child process.
			*/
			PyOS_snprintf(
				s2, x,
				"\"%s\" %s%s%s",
				modulepath,
				s1,
				s3,
				cmdstring);
			/* Not passing CREATE_NEW_CONSOLE has been known to
			   cause random failures on win9x.  Specifically a 
			   dialog:
			   "Your program accessed mem currently in use at xxx"
			   and a hopeful warning about the stability of your
			   system.
			   Cost is Ctrl+C wont kill children, but anyone
			   who cares can have a go!
			*/
			dwProcessFlags |= CREATE_NEW_CONSOLE;
		}
	}

	/* Could be an else here to try cmd.exe / command.com in the path
	   Now we'll just error out.. */
	else {
		PyErr_SetString(PyExc_RuntimeError,
			"Cannot locate a COMSPEC environment variable to "
			"use as the shell");
		return FALSE;
	}

	ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	siStartInfo.hStdInput = hStdin;
	siStartInfo.hStdOutput = hStdout;
	siStartInfo.hStdError = hStderr;
	siStartInfo.wShowWindow = SW_HIDE;

	if (CreateProcess(NULL,
			  s2,
			  NULL,
			  NULL,
			  TRUE,
			  dwProcessFlags,
			  NULL,
			  NULL,
			  &siStartInfo,
			  &piProcInfo) ) {
		/* Close the handles now so anyone waiting is woken. */
		CloseHandle(piProcInfo.hThread);

		/* Return process handle */
		*hProcess = piProcInfo.hProcess;
		return TRUE;
	}
	win32_error("CreateProcess", s2);
	return FALSE;
}

/* The following code is based off of KB: Q190351 */

static PyObject *
_PyPopen(char *cmdstring, int mode, int n)
{
	HANDLE hChildStdinRd, hChildStdinWr, hChildStdoutRd, hChildStdoutWr,
		hChildStderrRd, hChildStderrWr, hChildStdinWrDup, hChildStdoutRdDup,
		hChildStderrRdDup, hProcess; /* hChildStdoutWrDup; */

	SECURITY_ATTRIBUTES saAttr;
	BOOL fSuccess;
	int fd1, fd2, fd3;
	FILE *f1, *f2, *f3;
	long file_count;
	PyObject *f;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0))
		return win32_error("CreatePipe", NULL);

	/* Create new output read handle and the input write handle. Set
	 * the inheritance properties to FALSE. Otherwise, the child inherits
	 * the these handles; resulting in non-closeable handles to the pipes
	 * being created. */
	 fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdinWr,
				    GetCurrentProcess(), &hChildStdinWrDup, 0,
				    FALSE,
				    DUPLICATE_SAME_ACCESS);
	 if (!fSuccess)
		 return win32_error("DuplicateHandle", NULL);

	 /* Close the inheritable version of ChildStdin
	that we're using. */
	 CloseHandle(hChildStdinWr);

	 if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))
		 return win32_error("CreatePipe", NULL);

	 fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdoutRd,
				    GetCurrentProcess(), &hChildStdoutRdDup, 0,
				    FALSE, DUPLICATE_SAME_ACCESS);
	 if (!fSuccess)
		 return win32_error("DuplicateHandle", NULL);

	 /* Close the inheritable version of ChildStdout
		that we're using. */
	 CloseHandle(hChildStdoutRd);

	 if (n != POPEN_4) {
		 if (!CreatePipe(&hChildStderrRd, &hChildStderrWr, &saAttr, 0))
			 return win32_error("CreatePipe", NULL);
		 fSuccess = DuplicateHandle(GetCurrentProcess(),
					    hChildStderrRd,
					    GetCurrentProcess(),
					    &hChildStderrRdDup, 0,
					    FALSE, DUPLICATE_SAME_ACCESS);
		 if (!fSuccess)
			 return win32_error("DuplicateHandle", NULL);
		 /* Close the inheritable version of ChildStdErr that we're using. */
		 CloseHandle(hChildStderrRd);
	 }

	 switch (n) {
	 case POPEN_1:
		 switch (mode & (_O_RDONLY | _O_TEXT | _O_BINARY | _O_WRONLY)) {
		 case _O_WRONLY | _O_TEXT:
			 /* Case for writing to child Stdin in text mode. */
			 fd1 = _open_osfhandle((long)hChildStdinWrDup, mode);
			 f1 = _fdopen(fd1, "w");
			 f = PyFile_FromFile(f1, cmdstring, "w", _PyPclose);
			 PyFile_SetBufSize(f, 0);
			 /* We don't care about these pipes anymore, so close them. */
			 CloseHandle(hChildStdoutRdDup);
			 CloseHandle(hChildStderrRdDup);
			 break;

		 case _O_RDONLY | _O_TEXT:
			 /* Case for reading from child Stdout in text mode. */
			 fd1 = _open_osfhandle((long)hChildStdoutRdDup, mode);
			 f1 = _fdopen(fd1, "r");
			 f = PyFile_FromFile(f1, cmdstring, "r", _PyPclose);
			 PyFile_SetBufSize(f, 0);
			 /* We don't care about these pipes anymore, so close them. */
			 CloseHandle(hChildStdinWrDup);
			 CloseHandle(hChildStderrRdDup);
			 break;

		 case _O_RDONLY | _O_BINARY:
			 /* Case for readinig from child Stdout in binary mode. */
			 fd1 = _open_osfhandle((long)hChildStdoutRdDup, mode);
			 f1 = _fdopen(fd1, "rb");
			 f = PyFile_FromFile(f1, cmdstring, "rb", _PyPclose);
			 PyFile_SetBufSize(f, 0);
			 /* We don't care about these pipes anymore, so close them. */
			 CloseHandle(hChildStdinWrDup);
			 CloseHandle(hChildStderrRdDup);
			 break;

		 case _O_WRONLY | _O_BINARY:
			 /* Case for writing to child Stdin in binary mode. */
			 fd1 = _open_osfhandle((long)hChildStdinWrDup, mode);
			 f1 = _fdopen(fd1, "wb");
			 f = PyFile_FromFile(f1, cmdstring, "wb", _PyPclose);
			 PyFile_SetBufSize(f, 0);
			 /* We don't care about these pipes anymore, so close them. */
			 CloseHandle(hChildStdoutRdDup);
			 CloseHandle(hChildStderrRdDup);
			 break;
		 }
		 file_count = 1;
		 break;

	 case POPEN_2:
	 case POPEN_4:
	 {
		 char *m1, *m2;
		 PyObject *p1, *p2;

		 if (mode & _O_TEXT) {
			 m1 = "r";
			 m2 = "w";
		 } else {
			 m1 = "rb";
			 m2 = "wb";
		 }

		 fd1 = _open_osfhandle((long)hChildStdinWrDup, mode);
		 f1 = _fdopen(fd1, m2);
		 fd2 = _open_osfhandle((long)hChildStdoutRdDup, mode);
		 f2 = _fdopen(fd2, m1);
		 p1 = PyFile_FromFile(f1, cmdstring, m2, _PyPclose);
		 PyFile_SetBufSize(p1, 0);
		 p2 = PyFile_FromFile(f2, cmdstring, m1, _PyPclose);
		 PyFile_SetBufSize(p2, 0);

		 if (n != 4)
			 CloseHandle(hChildStderrRdDup);

		 f = Py_BuildValue("OO",p1,p2);
		 Py_XDECREF(p1);
		 Py_XDECREF(p2);
		 file_count = 2;
		 break;
	 }

	 case POPEN_3:
	 {
		 char *m1, *m2;
		 PyObject *p1, *p2, *p3;

		 if (mode & _O_TEXT) {
			 m1 = "r";
			 m2 = "w";
		 } else {
			 m1 = "rb";
			 m2 = "wb";
		 }

		 fd1 = _open_osfhandle((long)hChildStdinWrDup, mode);
		 f1 = _fdopen(fd1, m2);
		 fd2 = _open_osfhandle((long)hChildStdoutRdDup, mode);
		 f2 = _fdopen(fd2, m1);
		 fd3 = _open_osfhandle((long)hChildStderrRdDup, mode);
		 f3 = _fdopen(fd3, m1);
		 p1 = PyFile_FromFile(f1, cmdstring, m2, _PyPclose);
		 p2 = PyFile_FromFile(f2, cmdstring, m1, _PyPclose);
		 p3 = PyFile_FromFile(f3, cmdstring, m1, _PyPclose);
		 PyFile_SetBufSize(p1, 0);
		 PyFile_SetBufSize(p2, 0);
		 PyFile_SetBufSize(p3, 0);
		 f = Py_BuildValue("OOO",p1,p2,p3);
		 Py_XDECREF(p1);
		 Py_XDECREF(p2);
		 Py_XDECREF(p3);
		 file_count = 3;
		 break;
	 }
	 }

	 if (n == POPEN_4) {
		 if (!_PyPopenCreateProcess(cmdstring,
					    hChildStdinRd,
					    hChildStdoutWr,
					    hChildStdoutWr,
					    &hProcess))
			 return NULL;
	 }
	 else {
		 if (!_PyPopenCreateProcess(cmdstring,
					    hChildStdinRd,
					    hChildStdoutWr,
					    hChildStderrWr,
					    &hProcess))
			 return NULL;
	 }

	 /*
	  * Insert the files we've created into the process dictionary
	  * all referencing the list with the process handle and the
	  * initial number of files (see description below in _PyPclose).
	  * Since if _PyPclose later tried to wait on a process when all
	  * handles weren't closed, it could create a deadlock with the
	  * child, we spend some energy here to try to ensure that we
	  * either insert all file handles into the dictionary or none
	  * at all.  It's a little clumsy with the various popen modes
	  * and variable number of files involved.
	  */
	 if (!_PyPopenProcs) {
		 _PyPopenProcs = PyDict_New();
	 }

	 if (_PyPopenProcs) {
		 PyObject *procObj, *hProcessObj, *intObj, *fileObj[3];
		 int ins_rc[3];

		 fileObj[0] = fileObj[1] = fileObj[2] = NULL;
		 ins_rc[0]  = ins_rc[1]  = ins_rc[2]  = 0;

		 procObj = PyList_New(2);
		 hProcessObj = PyLong_FromVoidPtr(hProcess);
		 intObj = PyInt_FromLong(file_count);

		 if (procObj && hProcessObj && intObj) {
			 PyList_SetItem(procObj,0,hProcessObj);
			 PyList_SetItem(procObj,1,intObj);

			 fileObj[0] = PyLong_FromVoidPtr(f1);
			 if (fileObj[0]) {
			    ins_rc[0] = PyDict_SetItem(_PyPopenProcs,
						       fileObj[0],
						       procObj);
			 }
			 if (file_count >= 2) {
				 fileObj[1] = PyLong_FromVoidPtr(f2);
				 if (fileObj[1]) {
				    ins_rc[1] = PyDict_SetItem(_PyPopenProcs,
							       fileObj[1],
							       procObj);
				 }
			 }
			 if (file_count >= 3) {
				 fileObj[2] = PyLong_FromVoidPtr(f3);
				 if (fileObj[2]) {
				    ins_rc[2] = PyDict_SetItem(_PyPopenProcs,
							       fileObj[2],
							       procObj);
				 }
			 }

			 if (ins_rc[0] < 0 || !fileObj[0] ||
			     ins_rc[1] < 0 || (file_count > 1 && !fileObj[1]) ||
			     ins_rc[2] < 0 || (file_count > 2 && !fileObj[2])) {
				 /* Something failed - remove any dictionary
				  * entries that did make it.
				  */
				 if (!ins_rc[0] && fileObj[0]) {
					 PyDict_DelItem(_PyPopenProcs,
							fileObj[0]);
				 }
				 if (!ins_rc[1] && fileObj[1]) {
					 PyDict_DelItem(_PyPopenProcs,
							fileObj[1]);
				 }
				 if (!ins_rc[2] && fileObj[2]) {
					 PyDict_DelItem(_PyPopenProcs,
							fileObj[2]);
				 }
			 }
		 }

		 /*
		  * Clean up our localized references for the dictionary keys
		  * and value since PyDict_SetItem will Py_INCREF any copies
		  * that got placed in the dictionary.
		  */
		 Py_XDECREF(procObj);
		 Py_XDECREF(fileObj[0]);
		 Py_XDECREF(fileObj[1]);
		 Py_XDECREF(fileObj[2]);
	 }

	 /* Child is launched. Close the parents copy of those pipe
	  * handles that only the child should have open.  You need to
	  * make sure that no handles to the write end of the output pipe
	  * are maintained in this process or else the pipe will not close
	  * when the child process exits and the ReadFile will hang. */

	 if (!CloseHandle(hChildStdinRd))
		 return win32_error("CloseHandle", NULL);

	 if (!CloseHandle(hChildStdoutWr))
		 return win32_error("CloseHandle", NULL);

	 if ((n != 4) && (!CloseHandle(hChildStderrWr)))
		 return win32_error("CloseHandle", NULL);

	 return f;
}

/*
 * Wrapper for fclose() to use for popen* files, so we can retrieve the
 * exit code for the child process and return as a result of the close.
 *
 * This function uses the _PyPopenProcs dictionary in order to map the
 * input file pointer to information about the process that was
 * originally created by the popen* call that created the file pointer.
 * The dictionary uses the file pointer as a key (with one entry
 * inserted for each file returned by the original popen* call) and a
 * single list object as the value for all files from a single call.
 * The list object contains the Win32 process handle at [0], and a file
 * count at [1], which is initialized to the total number of file
 * handles using that list.
 *
 * This function closes whichever handle it is passed, and decrements
 * the file count in the dictionary for the process handle pointed to
 * by this file.  On the last close (when the file count reaches zero),
 * this function will wait for the child process and then return its
 * exit code as the result of the close() operation.  This permits the
 * files to be closed in any order - it is always the close() of the
 * final handle that will return the exit code.
 */

 /* RED_FLAG 31-Aug-2000 Tim
  * This is always called (today!) between a pair of
  * Py_BEGIN_ALLOW_THREADS/ Py_END_ALLOW_THREADS
  * macros.  So the thread running this has no valid thread state, as
  * far as Python is concerned.  However, this calls some Python API
  * functions that cannot be called safely without a valid thread
  * state, in particular PyDict_GetItem.
  * As a temporary hack (although it may last for years ...), we
  * *rely* on not having a valid thread state in this function, in
  * order to create our own "from scratch".
  * This will deadlock if _PyPclose is ever called by a thread
  * holding the global lock.
  */

static int _PyPclose(FILE *file)
{
	int result;
	DWORD exit_code;
	HANDLE hProcess;
	PyObject *procObj, *hProcessObj, *intObj, *fileObj;
	long file_count;
#ifdef WITH_THREAD
	PyInterpreterState* pInterpreterState;
	PyThreadState* pThreadState;
#endif

	/* Close the file handle first, to ensure it can't block the
	 * child from exiting if it's the last handle.
	 */
	result = fclose(file);

#ifdef WITH_THREAD
	/* Bootstrap a valid thread state into existence. */
	pInterpreterState = PyInterpreterState_New();
	if (!pInterpreterState) {
		/* Well, we're hosed now!  We don't have a thread
		 * state, so can't call a nice error routine, or raise
		 * an exception.  Just die.
		 */
		 Py_FatalError("unable to allocate interpreter state "
		 	       "when closing popen object");
		 return -1;  /* unreachable */
	}
	pThreadState = PyThreadState_New(pInterpreterState);
	if (!pThreadState) {
		 Py_FatalError("unable to allocate thread state "
		 	       "when closing popen object");
		 return -1;  /* unreachable */
	}
	/* Grab the global lock.  Note that this will deadlock if the
	 * current thread already has the lock! (see RED_FLAG comments
	 * before this function)
	 */
	PyEval_RestoreThread(pThreadState);
#endif

	if (_PyPopenProcs) {
		if ((fileObj = PyLong_FromVoidPtr(file)) != NULL &&
		    (procObj = PyDict_GetItem(_PyPopenProcs,
					      fileObj)) != NULL &&
		    (hProcessObj = PyList_GetItem(procObj,0)) != NULL &&
		    (intObj = PyList_GetItem(procObj,1)) != NULL) {

			hProcess = PyLong_AsVoidPtr(hProcessObj);
			file_count = PyInt_AsLong(intObj);

			if (file_count > 1) {
				/* Still other files referencing process */
				file_count--;
				PyList_SetItem(procObj,1,
					       PyInt_FromLong(file_count));
			} else {
				/* Last file for this process */
				if (result != EOF &&
				    WaitForSingleObject(hProcess, INFINITE) != WAIT_FAILED &&
				    GetExitCodeProcess(hProcess, &exit_code)) {
					/* Possible truncation here in 16-bit environments, but
					 * real exit codes are just the lower byte in any event.
					 */
					result = exit_code;
				} else {
					/* Indicate failure - this will cause the file object
					 * to raise an I/O error and translate the last Win32
					 * error code from errno.  We do have a problem with
					 * last errors that overlap the normal errno table,
					 * but that's a consistent problem with the file object.
					 */
					if (result != EOF) {
						/* If the error wasn't from the fclose(), then
						 * set errno for the file object error handling.
						 */
						errno = GetLastError();
					}
					result = -1;
				}

				/* Free up the native handle at this point */
				CloseHandle(hProcess);
			}

			/* Remove this file pointer from dictionary */
			PyDict_DelItem(_PyPopenProcs, fileObj);

			if (PyDict_Size(_PyPopenProcs) == 0) {
				Py_DECREF(_PyPopenProcs);
				_PyPopenProcs = NULL;
			}

		} /* if object retrieval ok */

		Py_XDECREF(fileObj);
	} /* if _PyPopenProcs */

#ifdef WITH_THREAD
	/* Tear down the thread & interpreter states.
	 * Note that interpreter state clear & delete functions automatically
	 * call the thread clear & delete functions, and indeed insist on
	 * doing that themselves.  The lock must be held during the clear, but
	 * need not be held during the delete.
	 */
	PyInterpreterState_Clear(pInterpreterState);
	PyEval_ReleaseThread(pThreadState);
	PyInterpreterState_Delete(pInterpreterState);
#endif

	return result;
}

#else /* which OS? */
static PyObject *
posix_popen(PyObject *self, PyObject *args)
{
	char *name;
	char *mode = "r";
	int bufsize = -1;
	FILE *fp;
	PyObject *f;
	if (!PyArg_ParseTuple(args, "s|si:popen", &name, &mode, &bufsize))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	fp = popen(name, mode);
	Py_END_ALLOW_THREADS
	if (fp == NULL)
		return posix_error();
	f = PyFile_FromFile(fp, name, mode, pclose);
	if (f != NULL)
		PyFile_SetBufSize(f, bufsize);
	return f;
}

#endif /* PYOS_??? */
#endif /* HAVE_POPEN */


#ifdef HAVE_SETUID
PyDoc_STRVAR(posix_setuid__doc__,
"setuid(uid)\n\n\
Set the current process's user id.");

static PyObject *
posix_setuid(PyObject *self, PyObject *args)
{
	int uid;
	if (!PyArg_ParseTuple(args, "i:setuid", &uid))
		return NULL;
	if (setuid(uid) < 0)
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif /* HAVE_SETUID */


#ifdef HAVE_SETEUID
PyDoc_STRVAR(posix_seteuid__doc__,
"seteuid(uid)\n\n\
Set the current process's effective user id.");

static PyObject *
posix_seteuid (PyObject *self, PyObject *args)
{
	int euid;
	if (!PyArg_ParseTuple(args, "i", &euid)) {
		return NULL;
	} else if (seteuid(euid) < 0) {
		return posix_error();
	} else {
		Py_INCREF(Py_None);
		return Py_None;
	}
}
#endif /* HAVE_SETEUID */

#ifdef HAVE_SETEGID
PyDoc_STRVAR(posix_setegid__doc__,
"setegid(gid)\n\n\
Set the current process's effective group id.");

static PyObject *
posix_setegid (PyObject *self, PyObject *args)
{
	int egid;
	if (!PyArg_ParseTuple(args, "i", &egid)) {
		return NULL;
	} else if (setegid(egid) < 0) {
		return posix_error();
	} else {
		Py_INCREF(Py_None);
		return Py_None;
	}
}
#endif /* HAVE_SETEGID */

#ifdef HAVE_SETREUID
PyDoc_STRVAR(posix_setreuid__doc__,
"seteuid(ruid, euid)\n\n\
Set the current process's real and effective user ids.");

static PyObject *
posix_setreuid (PyObject *self, PyObject *args)
{
	int ruid, euid;
	if (!PyArg_ParseTuple(args, "ii", &ruid, &euid)) {
		return NULL;
	} else if (setreuid(ruid, euid) < 0) {
		return posix_error();
	} else {
		Py_INCREF(Py_None);
		return Py_None;
	}
}
#endif /* HAVE_SETREUID */

#ifdef HAVE_SETREGID
PyDoc_STRVAR(posix_setregid__doc__,
"setegid(rgid, egid)\n\n\
Set the current process's real and effective group ids.");

static PyObject *
posix_setregid (PyObject *self, PyObject *args)
{
	int rgid, egid;
	if (!PyArg_ParseTuple(args, "ii", &rgid, &egid)) {
		return NULL;
	} else if (setregid(rgid, egid) < 0) {
		return posix_error();
	} else {
		Py_INCREF(Py_None);
		return Py_None;
	}
}
#endif /* HAVE_SETREGID */

#ifdef HAVE_SETGID
PyDoc_STRVAR(posix_setgid__doc__,
"setgid(gid)\n\n\
Set the current process's group id.");

static PyObject *
posix_setgid(PyObject *self, PyObject *args)
{
	int gid;
	if (!PyArg_ParseTuple(args, "i:setgid", &gid))
		return NULL;
	if (setgid(gid) < 0)
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif /* HAVE_SETGID */

#ifdef HAVE_SETGROUPS
PyDoc_STRVAR(posix_setgroups__doc__,
"setgroups(list)\n\n\
Set the groups of the current process to list.");

static PyObject *
posix_setgroups(PyObject *self, PyObject *args)
{
	PyObject *groups;
	int i, len;
        gid_t grouplist[MAX_GROUPS];

	if (!PyArg_ParseTuple(args, "O:setgid", &groups))
		return NULL;
	if (!PySequence_Check(groups)) {
		PyErr_SetString(PyExc_TypeError, "setgroups argument must be a sequence");
		return NULL;
	}
	len = PySequence_Size(groups);
	if (len > MAX_GROUPS) {
		PyErr_SetString(PyExc_ValueError, "too many groups");
		return NULL;
	}
	for(i = 0; i < len; i++) {
		PyObject *elem;
		elem = PySequence_GetItem(groups, i);
		if (!elem)
			return NULL;
		if (!PyInt_Check(elem)) {
			PyErr_SetString(PyExc_TypeError,
					"groups must be integers");
			Py_DECREF(elem);
			return NULL;
		}
		/* XXX: check that value fits into gid_t. */
		grouplist[i] = PyInt_AsLong(elem);
		Py_DECREF(elem);
	}

	if (setgroups(len, grouplist) < 0)
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif /* HAVE_SETGROUPS */

#ifdef HAVE_WAITPID
PyDoc_STRVAR(posix_waitpid__doc__,
"waitpid(pid, options) -> (pid, status)\n\n\
Wait for completion of a given child process.");

static PyObject *
posix_waitpid(PyObject *self, PyObject *args)
{
	int pid, options;
#ifdef UNION_WAIT
	union wait status;
#define status_i (status.w_status)
#else
	int status;
#define status_i status
#endif
	status_i = 0;

	if (!PyArg_ParseTuple(args, "ii:waitpid", &pid, &options))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	pid = waitpid(pid, &status, options);
	Py_END_ALLOW_THREADS
	if (pid == -1)
		return posix_error();
	else
		return Py_BuildValue("ii", pid, status_i);
}

#elif defined(HAVE_CWAIT)

/* MS C has a variant of waitpid() that's usable for most purposes. */
PyDoc_STRVAR(posix_waitpid__doc__,
"waitpid(pid, options) -> (pid, status << 8)\n\n"
"Wait for completion of a given process.  options is ignored on Windows.");

static PyObject *
posix_waitpid(PyObject *self, PyObject *args)
{
	int pid, options;
	int status;

	if (!PyArg_ParseTuple(args, "ii:waitpid", &pid, &options))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	pid = _cwait(&status, pid, options);
	Py_END_ALLOW_THREADS
	if (pid == -1)
		return posix_error();
	else
		/* shift the status left a byte so this is more like the
		   POSIX waitpid */
		return Py_BuildValue("ii", pid, status << 8);
}
#endif /* HAVE_WAITPID || HAVE_CWAIT */

#ifdef HAVE_WAIT
PyDoc_STRVAR(posix_wait__doc__,
"wait() -> (pid, status)\n\n\
Wait for completion of a child process.");

static PyObject *
posix_wait(PyObject *self, PyObject *args)
{
	int pid;
#ifdef UNION_WAIT
	union wait status;
#define status_i (status.w_status)
#else
	int status;
#define status_i status
#endif
        if (!PyArg_ParseTuple(args, ":wait"))
                return NULL;
	status_i = 0;
	Py_BEGIN_ALLOW_THREADS
	pid = wait(&status);
	Py_END_ALLOW_THREADS
	if (pid == -1)
		return posix_error();
	else
		return Py_BuildValue("ii", pid, status_i);
#undef status_i
}
#endif


PyDoc_STRVAR(posix_lstat__doc__,
"lstat(path) -> stat result\n\n\
Like stat(path), but do not follow symbolic links.");

static PyObject *
posix_lstat(PyObject *self, PyObject *args)
{
#ifdef HAVE_LSTAT
	return posix_do_stat(self, args, "et:lstat", lstat, NULL, NULL);
#else /* !HAVE_LSTAT */
#ifdef MS_WINDOWS
	return posix_do_stat(self, args, "et:lstat", STAT, "u:lstat", _wstati64);
#else
	return posix_do_stat(self, args, "et:lstat", STAT, NULL, NULL);
#endif
#endif /* !HAVE_LSTAT */
}


#ifdef HAVE_READLINK
PyDoc_STRVAR(posix_readlink__doc__,
"readlink(path) -> path\n\n\
Return a string representing the path to which the symbolic link points.");

static PyObject *
posix_readlink(PyObject *self, PyObject *args)
{
	char buf[MAXPATHLEN];
	char *path;
	int n;
	if (!PyArg_ParseTuple(args, "s:readlink", &path))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	n = readlink(path, buf, (int) sizeof buf);
	Py_END_ALLOW_THREADS
	if (n < 0)
		return posix_error_with_filename(path);
	return PyString_FromStringAndSize(buf, n);
}
#endif /* HAVE_READLINK */


#ifdef HAVE_SYMLINK
PyDoc_STRVAR(posix_symlink__doc__,
"symlink(src, dst)\n\n\
Create a symbolic link.");

static PyObject *
posix_symlink(PyObject *self, PyObject *args)
{
	return posix_2str(args, "etet:symlink", symlink, NULL, NULL);
}
#endif /* HAVE_SYMLINK */


#ifdef HAVE_TIMES
#ifndef HZ
#define HZ 60 /* Universal constant :-) */
#endif /* HZ */

#if defined(PYCC_VACPP) && defined(PYOS_OS2)
static long
system_uptime(void)
{
    ULONG     value = 0;

    Py_BEGIN_ALLOW_THREADS
    DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, &value, sizeof(value));
    Py_END_ALLOW_THREADS

    return value;
}

static PyObject *
posix_times(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":times"))
		return NULL;

    /* Currently Only Uptime is Provided -- Others Later */
	return Py_BuildValue("ddddd",
			     (double)0 /* t.tms_utime / HZ */,
			     (double)0 /* t.tms_stime / HZ */,
			     (double)0 /* t.tms_cutime / HZ */,
			     (double)0 /* t.tms_cstime / HZ */,
			     (double)system_uptime() / 1000);
}
#else /* not OS2 */
static PyObject *
posix_times(PyObject *self, PyObject *args)
{
	struct tms t;
	clock_t c;
	if (!PyArg_ParseTuple(args, ":times"))
		return NULL;
	errno = 0;
	c = times(&t);
	if (c == (clock_t) -1)
		return posix_error();
	return Py_BuildValue("ddddd",
			     (double)t.tms_utime / HZ,
			     (double)t.tms_stime / HZ,
			     (double)t.tms_cutime / HZ,
			     (double)t.tms_cstime / HZ,
			     (double)c / HZ);
}
#endif /* not OS2 */
#endif /* HAVE_TIMES */


#ifdef MS_WINDOWS
#define HAVE_TIMES	/* so the method table will pick it up */
static PyObject *
posix_times(PyObject *self, PyObject *args)
{
	FILETIME create, exit, kernel, user;
	HANDLE hProc;
	if (!PyArg_ParseTuple(args, ":times"))
		return NULL;
	hProc = GetCurrentProcess();
	GetProcessTimes(hProc, &create, &exit, &kernel, &user);
	/* The fields of a FILETIME structure are the hi and lo part
	   of a 64-bit value expressed in 100 nanosecond units.
	   1e7 is one second in such units; 1e-7 the inverse.
	   429.4967296 is 2**32 / 1e7 or 2**32 * 1e-7.
	*/
	return Py_BuildValue(
		"ddddd",
		(double)(kernel.dwHighDateTime*429.4967296 +
		         kernel.dwLowDateTime*1e-7),
		(double)(user.dwHighDateTime*429.4967296 +
		         user.dwLowDateTime*1e-7),
		(double)0,
		(double)0,
		(double)0);
}
#endif /* MS_WINDOWS */

#ifdef HAVE_TIMES
PyDoc_STRVAR(posix_times__doc__,
"times() -> (utime, stime, cutime, cstime, elapsed_time)\n\n\
Return a tuple of floating point numbers indicating process times.");
#endif


#ifdef HAVE_SETSID
PyDoc_STRVAR(posix_setsid__doc__,
"setsid()\n\n\
Call the system call setsid().");

static PyObject *
posix_setsid(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args, ":setsid"))
		return NULL;
	if (setsid() < 0)
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif /* HAVE_SETSID */

#ifdef HAVE_SETPGID
PyDoc_STRVAR(posix_setpgid__doc__,
"setpgid(pid, pgrp)\n\n\
Call the system call setpgid().");

static PyObject *
posix_setpgid(PyObject *self, PyObject *args)
{
	int pid, pgrp;
	if (!PyArg_ParseTuple(args, "ii:setpgid", &pid, &pgrp))
		return NULL;
	if (setpgid(pid, pgrp) < 0)
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif /* HAVE_SETPGID */


#ifdef HAVE_TCGETPGRP
PyDoc_STRVAR(posix_tcgetpgrp__doc__,
"tcgetpgrp(fd) -> pgid\n\n\
Return the process group associated with the terminal given by a fd.");

static PyObject *
posix_tcgetpgrp(PyObject *self, PyObject *args)
{
	int fd, pgid;
	if (!PyArg_ParseTuple(args, "i:tcgetpgrp", &fd))
		return NULL;
	pgid = tcgetpgrp(fd);
	if (pgid < 0)
		return posix_error();
	return PyInt_FromLong((long)pgid);
}
#endif /* HAVE_TCGETPGRP */


#ifdef HAVE_TCSETPGRP
PyDoc_STRVAR(posix_tcsetpgrp__doc__,
"tcsetpgrp(fd, pgid)\n\n\
Set the process group associated with the terminal given by a fd.");

static PyObject *
posix_tcsetpgrp(PyObject *self, PyObject *args)
{
	int fd, pgid;
	if (!PyArg_ParseTuple(args, "ii:tcsetpgrp", &fd, &pgid))
		return NULL;
	if (tcsetpgrp(fd, pgid) < 0)
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif /* HAVE_TCSETPGRP */

/* Functions acting on file descriptors */

PyDoc_STRVAR(posix_open__doc__,
"open(filename, flag [, mode=0777]) -> fd\n\n\
Open a file (for low level IO).");

static PyObject *
posix_open(PyObject *self, PyObject *args)
{
	char *file = NULL;
	int flag;
	int mode = 0777;
	int fd;

#ifdef MS_WINDOWS
	if (unicode_file_names()) {
		PyUnicodeObject *po;
		if (PyArg_ParseTuple(args, "Ui|i:mkdir", &po, &flag, &mode)) {
			Py_BEGIN_ALLOW_THREADS
			/* PyUnicode_AS_UNICODE OK without thread
			   lock as it is a simple dereference. */
			fd = _wopen(PyUnicode_AS_UNICODE(po), flag, mode);
			Py_END_ALLOW_THREADS
			if (fd < 0)
				return posix_error();
			return PyInt_FromLong((long)fd);
		}
		/* Drop the argument parsing error as narrow strings
		   are also valid. */
		PyErr_Clear();
	}
#endif

	if (!PyArg_ParseTuple(args, "eti|i",
	                      Py_FileSystemDefaultEncoding, &file,
	                      &flag, &mode))
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	fd = open(file, flag, mode);
	Py_END_ALLOW_THREADS
	if (fd < 0)
		return posix_error_with_allocated_filename(file);
	PyMem_Free(file);
	return PyInt_FromLong((long)fd);
}


PyDoc_STRVAR(posix_close__doc__,
"close(fd)\n\n\
Close a file descriptor (for low level IO).");

static PyObject *
posix_close(PyObject *self, PyObject *args)
{
	int fd, res;
	if (!PyArg_ParseTuple(args, "i:close", &fd))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = close(fd);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}


PyDoc_STRVAR(posix_dup__doc__,
"dup(fd) -> fd2\n\n\
Return a duplicate of a file descriptor.");

static PyObject *
posix_dup(PyObject *self, PyObject *args)
{
	int fd;
	if (!PyArg_ParseTuple(args, "i:dup", &fd))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	fd = dup(fd);
	Py_END_ALLOW_THREADS
	if (fd < 0)
		return posix_error();
	return PyInt_FromLong((long)fd);
}


PyDoc_STRVAR(posix_dup2__doc__,
"dup2(fd, fd2)\n\n\
Duplicate file descriptor.");

static PyObject *
posix_dup2(PyObject *self, PyObject *args)
{
	int fd, fd2, res;
	if (!PyArg_ParseTuple(args, "ii:dup2", &fd, &fd2))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = dup2(fd, fd2);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}


PyDoc_STRVAR(posix_lseek__doc__,
"lseek(fd, pos, how) -> newpos\n\n\
Set the current position of a file descriptor.");

static PyObject *
posix_lseek(PyObject *self, PyObject *args)
{
	int fd, how;
#if defined(MS_WIN64) || defined(MS_WINDOWS)
	LONG_LONG pos, res;
#else
	off_t pos, res;
#endif
	PyObject *posobj;
	if (!PyArg_ParseTuple(args, "iOi:lseek", &fd, &posobj, &how))
		return NULL;
#ifdef SEEK_SET
	/* Turn 0, 1, 2 into SEEK_{SET,CUR,END} */
	switch (how) {
	case 0: how = SEEK_SET; break;
	case 1: how = SEEK_CUR; break;
	case 2: how = SEEK_END; break;
	}
#endif /* SEEK_END */

#if !defined(HAVE_LARGEFILE_SUPPORT)
	pos = PyInt_AsLong(posobj);
#else
	pos = PyLong_Check(posobj) ?
		PyLong_AsLongLong(posobj) : PyInt_AsLong(posobj);
#endif
	if (PyErr_Occurred())
		return NULL;

	Py_BEGIN_ALLOW_THREADS
#if defined(MS_WIN64) || defined(MS_WINDOWS)
	res = _lseeki64(fd, pos, how);
#else
	res = lseek(fd, pos, how);
#endif
	Py_END_ALLOW_THREADS
	if (res < 0)
		return posix_error();

#if !defined(HAVE_LARGEFILE_SUPPORT)
	return PyInt_FromLong(res);
#else
	return PyLong_FromLongLong(res);
#endif
}


PyDoc_STRVAR(posix_read__doc__,
"read(fd, buffersize) -> string\n\n\
Read a file descriptor.");

static PyObject *
posix_read(PyObject *self, PyObject *args)
{
	int fd, size, n;
	PyObject *buffer;
	if (!PyArg_ParseTuple(args, "ii:read", &fd, &size))
		return NULL;
	buffer = PyString_FromStringAndSize((char *)NULL, size);
	if (buffer == NULL)
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	n = read(fd, PyString_AsString(buffer), size);
	Py_END_ALLOW_THREADS
	if (n < 0) {
		Py_DECREF(buffer);
		return posix_error();
	}
	if (n != size)
		_PyString_Resize(&buffer, n);
	return buffer;
}


PyDoc_STRVAR(posix_write__doc__,
"write(fd, string) -> byteswritten\n\n\
Write a string to a file descriptor.");

static PyObject *
posix_write(PyObject *self, PyObject *args)
{
	int fd, size;
	char *buffer;
	if (!PyArg_ParseTuple(args, "is#:write", &fd, &buffer, &size))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	size = write(fd, buffer, size);
	Py_END_ALLOW_THREADS
	if (size < 0)
		return posix_error();
	return PyInt_FromLong((long)size);
}


PyDoc_STRVAR(posix_fstat__doc__,
"fstat(fd) -> stat result\n\n\
Like stat(), but for an open file descriptor.");

static PyObject *
posix_fstat(PyObject *self, PyObject *args)
{
	int fd;
	STRUCT_STAT st;
	int res;
	if (!PyArg_ParseTuple(args, "i:fstat", &fd))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = FSTAT(fd, &st);
	Py_END_ALLOW_THREADS
	if (res != 0)
		return posix_error();

	return _pystat_fromstructstat(st);
}


PyDoc_STRVAR(posix_fdopen__doc__,
"fdopen(fd, [, mode='r' [, bufsize]]) -> file_object\n\n\
Return an open file object connected to a file descriptor.");

static PyObject *
posix_fdopen(PyObject *self, PyObject *args)
{
	int fd;
	char *mode = "r";
	int bufsize = -1;
	FILE *fp;
	PyObject *f;
	if (!PyArg_ParseTuple(args, "i|si", &fd, &mode, &bufsize))
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	fp = fdopen(fd, mode);
	Py_END_ALLOW_THREADS
	if (fp == NULL)
		return posix_error();
	f = PyFile_FromFile(fp, "<fdopen>", mode, fclose);
	if (f != NULL)
		PyFile_SetBufSize(f, bufsize);
	return f;
}

PyDoc_STRVAR(posix_isatty__doc__,
"isatty(fd) -> bool\n\n\
Return True if the file descriptor 'fd' is an open file descriptor\n\
connected to the slave end of a terminal.");

static PyObject *
posix_isatty(PyObject *self, PyObject *args)
{
	int fd;
	if (!PyArg_ParseTuple(args, "i:isatty", &fd))
		return NULL;
	return PyBool_FromLong(isatty(fd));
}

#ifdef HAVE_PIPE
PyDoc_STRVAR(posix_pipe__doc__,
"pipe() -> (read_end, write_end)\n\n\
Create a pipe.");

static PyObject *
posix_pipe(PyObject *self, PyObject *args)
{
#if defined(PYOS_OS2)
    HFILE read, write;
    APIRET rc;

    if (!PyArg_ParseTuple(args, ":pipe"))
        return NULL;

	Py_BEGIN_ALLOW_THREADS
    rc = DosCreatePipe( &read, &write, 4096);
	Py_END_ALLOW_THREADS
    if (rc != NO_ERROR)
        return os2_error(rc);

    return Py_BuildValue("(ii)", read, write);
#else
#if !defined(MS_WINDOWS)
	int fds[2];
	int res;
	if (!PyArg_ParseTuple(args, ":pipe"))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = pipe(fds);
	Py_END_ALLOW_THREADS
	if (res != 0)
		return posix_error();
	return Py_BuildValue("(ii)", fds[0], fds[1]);
#else /* MS_WINDOWS */
	HANDLE read, write;
	int read_fd, write_fd;
	BOOL ok;
	if (!PyArg_ParseTuple(args, ":pipe"))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	ok = CreatePipe(&read, &write, NULL, 0);
	Py_END_ALLOW_THREADS
	if (!ok)
		return win32_error("CreatePipe", NULL);
	read_fd = _open_osfhandle((Py_intptr_t)read, 0);
	write_fd = _open_osfhandle((Py_intptr_t)write, 1);
	return Py_BuildValue("(ii)", read_fd, write_fd);
#endif /* MS_WINDOWS */
#endif
}
#endif  /* HAVE_PIPE */


#ifdef HAVE_MKFIFO
PyDoc_STRVAR(posix_mkfifo__doc__,
"mkfifo(filename, [, mode=0666])\n\n\
Create a FIFO (a POSIX named pipe).");

static PyObject *
posix_mkfifo(PyObject *self, PyObject *args)
{
	char *filename;
	int mode = 0666;
	int res;
	if (!PyArg_ParseTuple(args, "s|i:mkfifo", &filename, &mode))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = mkfifo(filename, mode);
	Py_END_ALLOW_THREADS
	if (res < 0)
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif


#if defined(HAVE_MKNOD) && defined(HAVE_MAKEDEV)
PyDoc_STRVAR(posix_mknod__doc__,
"mknod(filename, [, mode=0600, major, minor])\n\n\
Create a filesystem node (file, device special file or named pipe)\n\
named filename. mode specifies both the permissions to use and the\n\
type of node to be created, being combined (bitwise OR) with one of\n\
S_IFREG, S_IFCHR, S_IFBLK, and S_IFIFO. For S_IFCHR and S_IFBLK,\n\
major and minor define the newly created device special file, otherwise\n\
they are ignored.");


static PyObject *
posix_mknod(PyObject *self, PyObject *args)
{
	char *filename;
	int mode = 0600;
	int major = 0;
	int minor = 0;
	int res;
	if (!PyArg_ParseTuple(args, "s|iii:mknod", &filename,
			      &mode, &major, &minor))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = mknod(filename, mode, makedev(major, minor));
	Py_END_ALLOW_THREADS
	if (res < 0)
		return posix_error();
	Py_INCREF(Py_None);
	return Py_None;
}
#endif


#ifdef HAVE_FTRUNCATE
PyDoc_STRVAR(posix_ftruncate__doc__,
"ftruncate(fd, length)\n\n\
Truncate a file to a specified length.");

static PyObject *
posix_ftruncate(PyObject *self, PyObject *args)
{
	int fd;
	off_t length;
	int res;
	PyObject *lenobj;

	if (!PyArg_ParseTuple(args, "iO:ftruncate", &fd, &lenobj))
		return NULL;

#if !defined(HAVE_LARGEFILE_SUPPORT)
	length = PyInt_AsLong(lenobj);
#else
	length = PyLong_Check(lenobj) ?
		PyLong_AsLongLong(lenobj) : PyInt_AsLong(lenobj);
#endif
	if (PyErr_Occurred())
		return NULL;

	Py_BEGIN_ALLOW_THREADS
	res = ftruncate(fd, length);
	Py_END_ALLOW_THREADS
	if (res < 0) {
		PyErr_SetFromErrno(PyExc_IOError);
		return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}
#endif

#ifdef HAVE_PUTENV
PyDoc_STRVAR(posix_putenv__doc__,
"putenv(key, value)\n\n\
Change or add an environment variable.");

/* Save putenv() parameters as values here, so we can collect them when they
 * get re-set with another call for the same key. */
static PyObject *posix_putenv_garbage;

static PyObject *
posix_putenv(PyObject *self, PyObject *args)
{
        char *s1, *s2;
        char *new;
	PyObject *newstr;
	size_t len;

	if (!PyArg_ParseTuple(args, "ss:putenv", &s1, &s2))
		return NULL;

#if defined(PYOS_OS2)
    if (stricmp(s1, "BEGINLIBPATH") == 0) {
        APIRET rc;

        if (strlen(s2) == 0)  /* If New Value is an Empty String */
            s2 = NULL;        /* Then OS/2 API Wants a NULL to Undefine It */

        rc = DosSetExtLIBPATH(s2, BEGIN_LIBPATH);
        if (rc != NO_ERROR)
            return os2_error(rc);

    } else if (stricmp(s1, "ENDLIBPATH") == 0) {
        APIRET rc;

        if (strlen(s2) == 0)  /* If New Value is an Empty String */
            s2 = NULL;        /* Then OS/2 API Wants a NULL to Undefine It */

        rc = DosSetExtLIBPATH(s2, END_LIBPATH);
        if (rc != NO_ERROR)
            return os2_error(rc);
    } else {
#endif

	/* XXX This can leak memory -- not easy to fix :-( */
	len = strlen(s1) + strlen(s2) + 2;
	/* len includes space for a trailing \0; the size arg to
	   PyString_FromStringAndSize does not count that */
	newstr = PyString_FromStringAndSize(NULL, (int)len - 1);
	if (newstr == NULL)
		return PyErr_NoMemory();
	new = PyString_AS_STRING(newstr);
	PyOS_snprintf(new, len, "%s=%s", s1, s2);
	if (putenv(new)) {
                posix_error();
                return NULL;
	}
	/* Install the first arg and newstr in posix_putenv_garbage;
	 * this will cause previous value to be collected.  This has to
	 * happen after the real putenv() call because the old value
	 * was still accessible until then. */
	if (PyDict_SetItem(posix_putenv_garbage,
			   PyTuple_GET_ITEM(args, 0), newstr)) {
		/* really not much we can do; just leak */
		PyErr_Clear();
	}
	else {
		Py_DECREF(newstr);
	}

#if defined(PYOS_OS2)
    }
#endif
	Py_INCREF(Py_None);
        return Py_None;
}
#endif /* putenv */

#ifdef HAVE_UNSETENV
PyDoc_STRVAR(posix_unsetenv__doc__,
"unsetenv(key)\n\n\
Delete an environment variable.");

static PyObject *
posix_unsetenv(PyObject *self, PyObject *args)
{
        char *s1;

	if (!PyArg_ParseTuple(args, "s:unsetenv", &s1))
		return NULL;

	unsetenv(s1);

	/* Remove the key from posix_putenv_garbage;
	 * this will cause it to be collected.  This has to
	 * happen after the real unsetenv() call because the
	 * old value was still accessible until then.
	 */
	if (PyDict_DelItem(posix_putenv_garbage,
		PyTuple_GET_ITEM(args, 0))) {
		/* really not much we can do; just leak */
		PyErr_Clear();
	}

	Py_INCREF(Py_None);
	return Py_None;
}
#endif /* unsetenv */

#ifdef HAVE_STRERROR
PyDoc_STRVAR(posix_strerror__doc__,
"strerror(code) -> string\n\n\
Translate an error code to a message string.");

static PyObject *
posix_strerror(PyObject *self, PyObject *args)
{
	int code;
	char *message;
	if (!PyArg_ParseTuple(args, "i:strerror", &code))
		return NULL;
	message = strerror(code);
	if (message == NULL) {
		PyErr_SetString(PyExc_ValueError,
				"strerror() argument out of range");
		return NULL;
	}
	return PyString_FromString(message);
}
#endif /* strerror */


#ifdef HAVE_SYS_WAIT_H

#ifdef WCOREDUMP
PyDoc_STRVAR(posix_WCOREDUMP__doc__,
"WCOREDUMP(status) -> bool\n\n\
Return True if the process returning 'status' was dumped to a core file.");

static PyObject *
posix_WCOREDUMP(PyObject *self, PyObject *args)
{
#ifdef UNION_WAIT
	union wait status;
#define status_i (status.w_status)
#else
	int status;
#define status_i status
#endif
	status_i = 0;

	if (!PyArg_ParseTuple(args, "i:WCOREDUMP", &status_i))
	{
		return NULL;
	}

	return PyBool_FromLong(WCOREDUMP(status));
#undef status_i
}
#endif /* WCOREDUMP */

#ifdef WIFCONTINUED
PyDoc_STRVAR(posix_WIFCONTINUED__doc__,
"WIFCONTINUED(status) -> bool\n\n\
Return True if the process returning 'status' was continued from a\n\
job control stop.");

static PyObject *
posix_WIFCONTINUED(PyObject *self, PyObject *args)
{
#ifdef UNION_WAIT
	union wait status;
#define status_i (status.w_status)
#else
	int status;
#define status_i status
#endif
	status_i = 0;

	if (!PyArg_ParseTuple(args, "i:WCONTINUED", &status_i))
	{
		return NULL;
	}

	return PyBool_FromLong(WIFCONTINUED(status));
#undef status_i
}
#endif /* WIFCONTINUED */

#ifdef WIFSTOPPED
PyDoc_STRVAR(posix_WIFSTOPPED__doc__,
"WIFSTOPPED(status) -> bool\n\n\
Return True if the process returning 'status' was stopped.");

static PyObject *
posix_WIFSTOPPED(PyObject *self, PyObject *args)
{
#ifdef UNION_WAIT
	union wait status;
#define status_i (status.w_status)
#else
	int status;
#define status_i status
#endif
	status_i = 0;

	if (!PyArg_ParseTuple(args, "i:WIFSTOPPED", &status_i))
	{
		return NULL;
	}

	return PyBool_FromLong(WIFSTOPPED(status));
#undef status_i
}
#endif /* WIFSTOPPED */

#ifdef WIFSIGNALED
PyDoc_STRVAR(posix_WIFSIGNALED__doc__,
"WIFSIGNALED(status) -> bool\n\n\
Return True if the process returning 'status' was terminated by a signal.");

static PyObject *
posix_WIFSIGNALED(PyObject *self, PyObject *args)
{
#ifdef UNION_WAIT
	union wait status;
#define status_i (status.w_status)
#else
	int status;
#define status_i status
#endif
	status_i = 0;

	if (!PyArg_ParseTuple(args, "i:WIFSIGNALED", &status_i))
	{
		return NULL;
	}

	return PyBool_FromLong(WIFSIGNALED(status));
#undef status_i
}
#endif /* WIFSIGNALED */

#ifdef WIFEXITED
PyDoc_STRVAR(posix_WIFEXITED__doc__,
"WIFEXITED(status) -> bool\n\n\
Return true if the process returning 'status' exited using the exit()\n\
system call.");

static PyObject *
posix_WIFEXITED(PyObject *self, PyObject *args)
{
#ifdef UNION_WAIT
	union wait status;
#define status_i (status.w_status)
#else
	int status;
#define status_i status
#endif
	status_i = 0;

	if (!PyArg_ParseTuple(args, "i:WIFEXITED", &status_i))
	{
		return NULL;
	}

	return PyBool_FromLong(WIFEXITED(status));
#undef status_i
}
#endif /* WIFEXITED */

#ifdef WEXITSTATUS
PyDoc_STRVAR(posix_WEXITSTATUS__doc__,
"WEXITSTATUS(status) -> integer\n\n\
Return the process return code from 'status'.");

static PyObject *
posix_WEXITSTATUS(PyObject *self, PyObject *args)
{
#ifdef UNION_WAIT
	union wait status;
#define status_i (status.w_status)
#else
	int status;
#define status_i status
#endif
	status_i = 0;

	if (!PyArg_ParseTuple(args, "i:WEXITSTATUS", &status_i))
	{
		return NULL;
	}

	return Py_BuildValue("i", WEXITSTATUS(status));
#undef status_i
}
#endif /* WEXITSTATUS */

#ifdef WTERMSIG
PyDoc_STRVAR(posix_WTERMSIG__doc__,
"WTERMSIG(status) -> integer\n\n\
Return the signal that terminated the process that provided the 'status'\n\
value.");

static PyObject *
posix_WTERMSIG(PyObject *self, PyObject *args)
{
#ifdef UNION_WAIT
	union wait status;
#define status_i (status.w_status)
#else
	int status;
#define status_i status
#endif
	status_i = 0;

	if (!PyArg_ParseTuple(args, "i:WTERMSIG", &status_i))
	{
		return NULL;
	}

	return Py_BuildValue("i", WTERMSIG(status));
#undef status_i
}
#endif /* WTERMSIG */

#ifdef WSTOPSIG
PyDoc_STRVAR(posix_WSTOPSIG__doc__,
"WSTOPSIG(status) -> integer\n\n\
Return the signal that stopped the process that provided\n\
the 'status' value.");

static PyObject *
posix_WSTOPSIG(PyObject *self, PyObject *args)
{
#ifdef UNION_WAIT
	union wait status;
#define status_i (status.w_status)
#else
	int status;
#define status_i status
#endif
	status_i = 0;

	if (!PyArg_ParseTuple(args, "i:WSTOPSIG", &status_i))
	{
		return NULL;
	}

	return Py_BuildValue("i", WSTOPSIG(status));
#undef status_i
}
#endif /* WSTOPSIG */

#endif /* HAVE_SYS_WAIT_H */


#if defined(HAVE_FSTATVFS)
#ifdef _SCO_DS
/* SCO OpenServer 5.0 and later requires _SVID3 before it reveals the
   needed definitions in sys/statvfs.h */
#define _SVID3
#endif
#include <sys/statvfs.h>

static PyObject*
_pystatvfs_fromstructstatvfs(struct statvfs st) {
        PyObject *v = PyStructSequence_New(&StatVFSResultType);
	if (v == NULL)
		return NULL;

#if !defined(HAVE_LARGEFILE_SUPPORT)
        PyStructSequence_SET_ITEM(v, 0, PyInt_FromLong((long) st.f_bsize));
        PyStructSequence_SET_ITEM(v, 1, PyInt_FromLong((long) st.f_frsize));
        PyStructSequence_SET_ITEM(v, 2, PyInt_FromLong((long) st.f_blocks));
        PyStructSequence_SET_ITEM(v, 3, PyInt_FromLong((long) st.f_bfree));
        PyStructSequence_SET_ITEM(v, 4, PyInt_FromLong((long) st.f_bavail));
        PyStructSequence_SET_ITEM(v, 5, PyInt_FromLong((long) st.f_files));
        PyStructSequence_SET_ITEM(v, 6, PyInt_FromLong((long) st.f_ffree));
        PyStructSequence_SET_ITEM(v, 7, PyInt_FromLong((long) st.f_favail));
        PyStructSequence_SET_ITEM(v, 8, PyInt_FromLong((long) st.f_flag));
        PyStructSequence_SET_ITEM(v, 9, PyInt_FromLong((long) st.f_namemax));
#else
        PyStructSequence_SET_ITEM(v, 0, PyInt_FromLong((long) st.f_bsize));
        PyStructSequence_SET_ITEM(v, 1, PyInt_FromLong((long) st.f_frsize));
        PyStructSequence_SET_ITEM(v, 2,
			       PyLong_FromLongLong((LONG_LONG) st.f_blocks));
        PyStructSequence_SET_ITEM(v, 3,
			       PyLong_FromLongLong((LONG_LONG) st.f_bfree));
        PyStructSequence_SET_ITEM(v, 4,
			       PyLong_FromLongLong((LONG_LONG) st.f_bavail));
        PyStructSequence_SET_ITEM(v, 5,
			       PyLong_FromLongLong((LONG_LONG) st.f_files));
        PyStructSequence_SET_ITEM(v, 6,
			       PyLong_FromLongLong((LONG_LONG) st.f_ffree));
        PyStructSequence_SET_ITEM(v, 7,
			       PyLong_FromLongLong((LONG_LONG) st.f_favail));
        PyStructSequence_SET_ITEM(v, 8, PyInt_FromLong((long) st.f_flag));
        PyStructSequence_SET_ITEM(v, 9, PyInt_FromLong((long) st.f_namemax));
#endif

        return v;
}

PyDoc_STRVAR(posix_fstatvfs__doc__,
"fstatvfs(fd) -> statvfs result\n\n\
Perform an fstatvfs system call on the given fd.");

static PyObject *
posix_fstatvfs(PyObject *self, PyObject *args)
{
	int fd, res;
	struct statvfs st;

	if (!PyArg_ParseTuple(args, "i:fstatvfs", &fd))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = fstatvfs(fd, &st);
	Py_END_ALLOW_THREADS
	if (res != 0)
		return posix_error();

        return _pystatvfs_fromstructstatvfs(st);
}
#endif /* HAVE_FSTATVFS */


#if defined(HAVE_STATVFS)
#include <sys/statvfs.h>

PyDoc_STRVAR(posix_statvfs__doc__,
"statvfs(path) -> statvfs result\n\n\
Perform a statvfs system call on the given path.");

static PyObject *
posix_statvfs(PyObject *self, PyObject *args)
{
	char *path;
	int res;
	struct statvfs st;
	if (!PyArg_ParseTuple(args, "s:statvfs", &path))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	res = statvfs(path, &st);
	Py_END_ALLOW_THREADS
	if (res != 0)
		return posix_error_with_filename(path);

        return _pystatvfs_fromstructstatvfs(st);
}
#endif /* HAVE_STATVFS */


#ifdef HAVE_TEMPNAM
PyDoc_STRVAR(posix_tempnam__doc__,
"tempnam([dir[, prefix]]) -> string\n\n\
Return a unique name for a temporary file.\n\
The directory and a prefix may be specified as strings; they may be omitted\n\
or None if not needed.");

static PyObject *
posix_tempnam(PyObject *self, PyObject *args)
{
    PyObject *result = NULL;
    char *dir = NULL;
    char *pfx = NULL;
    char *name;

    if (!PyArg_ParseTuple(args, "|zz:tempnam", &dir, &pfx))
        return NULL;

    if (PyErr_Warn(PyExc_RuntimeWarning,
		  "tempnam is a potential security risk to your program") < 0)
	    return NULL;

#ifdef MS_WINDOWS
    name = _tempnam(dir, pfx);
#else
    name = tempnam(dir, pfx);
#endif
    if (name == NULL)
        return PyErr_NoMemory();
    result = PyString_FromString(name);
    free(name);
    return result;
}
#endif


#ifdef HAVE_TMPFILE
PyDoc_STRVAR(posix_tmpfile__doc__,
"tmpfile() -> file object\n\n\
Create a temporary file with no directory entries.");

static PyObject *
posix_tmpfile(PyObject *self, PyObject *args)
{
    FILE *fp;

    if (!PyArg_ParseTuple(args, ":tmpfile"))
        return NULL;
    fp = tmpfile();
    if (fp == NULL)
        return posix_error();
    return PyFile_FromFile(fp, "<tmpfile>", "w+b", fclose);
}
#endif


#ifdef HAVE_TMPNAM
PyDoc_STRVAR(posix_tmpnam__doc__,
"tmpnam() -> string\n\n\
Return a unique name for a temporary file.");

static PyObject *
posix_tmpnam(PyObject *self, PyObject *args)
{
    char buffer[L_tmpnam];
    char *name;

    if (!PyArg_ParseTuple(args, ":tmpnam"))
        return NULL;

    if (PyErr_Warn(PyExc_RuntimeWarning,
		  "tmpnam is a potential security risk to your program") < 0)
	    return NULL;

#ifdef USE_TMPNAM_R
    name = tmpnam_r(buffer);
#else
    name = tmpnam(buffer);
#endif
    if (name == NULL) {
        PyErr_SetObject(PyExc_OSError,
                        Py_BuildValue("is", 0,
#ifdef USE_TMPNAM_R
                                      "unexpected NULL from tmpnam_r"
#else
                                      "unexpected NULL from tmpnam"
#endif
                                      ));
        return NULL;
    }
    return PyString_FromString(buffer);
}
#endif


/* This is used for fpathconf(), pathconf(), confstr() and sysconf().
 * It maps strings representing configuration variable names to
 * integer values, allowing those functions to be called with the
 * magic names instead of polluting the module's namespace with tons of
 * rarely-used constants.  There are three separate tables that use
 * these definitions.
 *
 * This code is always included, even if none of the interfaces that
 * need it are included.  The #if hackery needed to avoid it would be
 * sufficiently pervasive that it's not worth the loss of readability.
 */
struct constdef {
    char *name;
    long value;
};

static int
conv_confname(PyObject *arg, int *valuep, struct constdef *table,
	      size_t tablesize)
{
    if (PyInt_Check(arg)) {
        *valuep = PyInt_AS_LONG(arg);
        return 1;
    }
    if (PyString_Check(arg)) {
        /* look up the value in the table using a binary search */
        size_t lo = 0;
		size_t mid;
        size_t hi = tablesize;
        int cmp;
        char *confname = PyString_AS_STRING(arg);
        while (lo < hi) {
            mid = (lo + hi) / 2;
            cmp = strcmp(confname, table[mid].name);
            if (cmp < 0)
                hi = mid;
            else if (cmp > 0)
                lo = mid + 1;
            else {
                *valuep = table[mid].value;
                return 1;
            }
        }
        PyErr_SetString(PyExc_ValueError, "unrecognized configuration name");
    }
    else
        PyErr_SetString(PyExc_TypeError,
                        "configuration names must be strings or integers");
    return 0;
}


#if defined(HAVE_FPATHCONF) || defined(HAVE_PATHCONF)
static struct constdef  posix_constants_pathconf[] = {
#ifdef _PC_ABI_AIO_XFER_MAX
    {"PC_ABI_AIO_XFER_MAX",	_PC_ABI_AIO_XFER_MAX},
#endif
#ifdef _PC_ABI_ASYNC_IO
    {"PC_ABI_ASYNC_IO",	_PC_ABI_ASYNC_IO},
#endif
#ifdef _PC_ASYNC_IO
    {"PC_ASYNC_IO",	_PC_ASYNC_IO},
#endif
#ifdef _PC_CHOWN_RESTRICTED
    {"PC_CHOWN_RESTRICTED",	_PC_CHOWN_RESTRICTED},
#endif
#ifdef _PC_FILESIZEBITS
    {"PC_FILESIZEBITS",	_PC_FILESIZEBITS},
#endif
#ifdef _PC_LAST
    {"PC_LAST",	_PC_LAST},
#endif
#ifdef _PC_LINK_MAX
    {"PC_LINK_MAX",	_PC_LINK_MAX},
#endif
#ifdef _PC_MAX_CANON
    {"PC_MAX_CANON",	_PC_MAX_CANON},
#endif
#ifdef _PC_MAX_INPUT
    {"PC_MAX_INPUT",	_PC_MAX_INPUT},
#endif
#ifdef _PC_NAME_MAX
    {"PC_NAME_MAX",	_PC_NAME_MAX},
#endif
#ifdef _PC_NO_TRUNC
    {"PC_NO_TRUNC",	_PC_NO_TRUNC},
#endif
#ifdef _PC_PATH_MAX
    {"PC_PATH_MAX",	_PC_PATH_MAX},
#endif
#ifdef _PC_PIPE_BUF
    {"PC_PIPE_BUF",	_PC_PIPE_BUF},
#endif
#ifdef _PC_PRIO_IO
    {"PC_PRIO_IO",	_PC_PRIO_IO},
#endif
#ifdef _PC_SOCK_MAXBUF
    {"PC_SOCK_MAXBUF",	_PC_SOCK_MAXBUF},
#endif
#ifdef _PC_SYNC_IO
    {"PC_SYNC_IO",	_PC_SYNC_IO},
#endif
#ifdef _PC_VDISABLE
    {"PC_VDISABLE",	_PC_VDISABLE},
#endif
};

static int
conv_path_confname(PyObject *arg, int *valuep)
{
    return conv_confname(arg, valuep, posix_constants_pathconf,
                         sizeof(posix_constants_pathconf)
                           / sizeof(struct constdef));
}
#endif

#ifdef HAVE_FPATHCONF
PyDoc_STRVAR(posix_fpathconf__doc__,
"fpathconf(fd, name) -> integer\n\n\
Return the configuration limit name for the file descriptor fd.\n\
If there is no limit, return -1.");

static PyObject *
posix_fpathconf(PyObject *self, PyObject *args)
{
    PyObject *result = NULL;
    int name, fd;

    if (PyArg_ParseTuple(args, "iO&:fpathconf", &fd,
                         conv_path_confname, &name)) {
        long limit;

        errno = 0;
        limit = fpathconf(fd, name);
        if (limit == -1 && errno != 0)
            posix_error();
        else
            result = PyInt_FromLong(limit);
    }
    return result;
}
#endif


#ifdef HAVE_PATHCONF
PyDoc_STRVAR(posix_pathconf__doc__,
"pathconf(path, name) -> integer\n\n\
Return the configuration limit name for the file or directory path.\n\
If there is no limit, return -1.");

static PyObject *
posix_pathconf(PyObject *self, PyObject *args)
{
    PyObject *result = NULL;
    int name;
    char *path;

    if (PyArg_ParseTuple(args, "sO&:pathconf", &path,
                         conv_path_confname, &name)) {
        long limit;

        errno = 0;
        limit = pathconf(path, name);
        if (limit == -1 && errno != 0) {
            if (errno == EINVAL)
                /* could be a path or name problem */
                posix_error();
            else
                posix_error_with_filename(path);
        }
        else
            result = PyInt_FromLong(limit);
    }
    return result;
}
#endif

#ifdef HAVE_CONFSTR
static struct constdef posix_constants_confstr[] = {
#ifdef _CS_ARCHITECTURE
    {"CS_ARCHITECTURE",	_CS_ARCHITECTURE},
#endif
#ifdef _CS_HOSTNAME
    {"CS_HOSTNAME",	_CS_HOSTNAME},
#endif
#ifdef _CS_HW_PROVIDER
    {"CS_HW_PROVIDER",	_CS_HW_PROVIDER},
#endif
#ifdef _CS_HW_SERIAL
    {"CS_HW_SERIAL",	_CS_HW_SERIAL},
#endif
#ifdef _CS_INITTAB_NAME
    {"CS_INITTAB_NAME",	_CS_INITTAB_NAME},
#endif
#ifdef _CS_LFS64_CFLAGS
    {"CS_LFS64_CFLAGS",	_CS_LFS64_CFLAGS},
#endif
#ifdef _CS_LFS64_LDFLAGS
    {"CS_LFS64_LDFLAGS",	_CS_LFS64_LDFLAGS},
#endif
#ifdef _CS_LFS64_LIBS
    {"CS_LFS64_LIBS",	_CS_LFS64_LIBS},
#endif
#ifdef _CS_LFS64_LINTFLAGS
    {"CS_LFS64_LINTFLAGS",	_CS_LFS64_LINTFLAGS},
#endif
#ifdef _CS_LFS_CFLAGS
    {"CS_LFS_CFLAGS",	_CS_LFS_CFLAGS},
#endif
#ifdef _CS_LFS_LDFLAGS
    {"CS_LFS_LDFLAGS",	_CS_LFS_LDFLAGS},
#endif
#ifdef _CS_LFS_LIBS
    {"CS_LFS_LIBS",	_CS_LFS_LIBS},
#endif
#ifdef _CS_LFS_LINTFLAGS
    {"CS_LFS_LINTFLAGS",	_CS_LFS_LINTFLAGS},
#endif
#ifdef _CS_MACHINE
    {"CS_MACHINE",	_CS_MACHINE},
#endif
#ifdef _CS_PATH
    {"CS_PATH",	_CS_PATH},
#endif
#ifdef _CS_RELEASE
    {"CS_RELEASE",	_CS_RELEASE},
#endif
#ifdef _CS_SRPC_DOMAIN
    {"CS_SRPC_DOMAIN",	_CS_SRPC_DOMAIN},
#endif
#ifdef _CS_SYSNAME
    {"CS_SYSNAME",	_CS_SYSNAME},
#endif
#ifdef _CS_VERSION
    {"CS_VERSION",	_CS_VERSION},
#endif
#ifdef _CS_XBS5_ILP32_OFF32_CFLAGS
    {"CS_XBS5_ILP32_OFF32_CFLAGS",	_CS_XBS5_ILP32_OFF32_CFLAGS},
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LDFLAGS
    {"CS_XBS5_ILP32_OFF32_LDFLAGS",	_CS_XBS5_ILP32_OFF32_LDFLAGS},
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LIBS
    {"CS_XBS5_ILP32_OFF32_LIBS",	_CS_XBS5_ILP32_OFF32_LIBS},
#endif
#ifdef _CS_XBS5_ILP32_OFF32_LINTFLAGS
    {"CS_XBS5_ILP32_OFF32_LINTFLAGS",	_CS_XBS5_ILP32_OFF32_LINTFLAGS},
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_CFLAGS
    {"CS_XBS5_ILP32_OFFBIG_CFLAGS",	_CS_XBS5_ILP32_OFFBIG_CFLAGS},
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LDFLAGS
    {"CS_XBS5_ILP32_OFFBIG_LDFLAGS",	_CS_XBS5_ILP32_OFFBIG_LDFLAGS},
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LIBS
    {"CS_XBS5_ILP32_OFFBIG_LIBS",	_CS_XBS5_ILP32_OFFBIG_LIBS},
#endif
#ifdef _CS_XBS5_ILP32_OFFBIG_LINTFLAGS
    {"CS_XBS5_ILP32_OFFBIG_LINTFLAGS",	_CS_XBS5_ILP32_OFFBIG_LINTFLAGS},
#endif
#ifdef _CS_XBS5_LP64_OFF64_CFLAGS
    {"CS_XBS5_LP64_OFF64_CFLAGS",	_CS_XBS5_LP64_OFF64_CFLAGS},
#endif
#ifdef _CS_XBS5_LP64_OFF64_LDFLAGS
    {"CS_XBS5_LP64_OFF64_LDFLAGS",	_CS_XBS5_LP64_OFF64_LDFLAGS},
#endif
#ifdef _CS_XBS5_LP64_OFF64_LIBS
    {"CS_XBS5_LP64_OFF64_LIBS",	_CS_XBS5_LP64_OFF64_LIBS},
#endif
#ifdef _CS_XBS5_LP64_OFF64_LINTFLAGS
    {"CS_XBS5_LP64_OFF64_LINTFLAGS",	_CS_XBS5_LP64_OFF64_LINTFLAGS},
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_CFLAGS
    {"CS_XBS5_LPBIG_OFFBIG_CFLAGS",	_CS_XBS5_LPBIG_OFFBIG_CFLAGS},
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LDFLAGS
    {"CS_XBS5_LPBIG_OFFBIG_LDFLAGS",	_CS_XBS5_LPBIG_OFFBIG_LDFLAGS},
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LIBS
    {"CS_XBS5_LPBIG_OFFBIG_LIBS",	_CS_XBS5_LPBIG_OFFBIG_LIBS},
#endif
#ifdef _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS
    {"CS_XBS5_LPBIG_OFFBIG_LINTFLAGS",	_CS_XBS5_LPBIG_OFFBIG_LINTFLAGS},
#endif
#ifdef _MIPS_CS_AVAIL_PROCESSORS
    {"MIPS_CS_AVAIL_PROCESSORS",	_MIPS_CS_AVAIL_PROCESSORS},
#endif
#ifdef _MIPS_CS_BASE
    {"MIPS_CS_BASE",	_MIPS_CS_BASE},
#endif
#ifdef _MIPS_CS_HOSTID
    {"MIPS_CS_HOSTID",	_MIPS_CS_HOSTID},
#endif
#ifdef _MIPS_CS_HW_NAME
    {"MIPS_CS_HW_NAME",	_MIPS_CS_HW_NAME},
#endif
#ifdef _MIPS_CS_NUM_PROCESSORS
    {"MIPS_CS_NUM_PROCESSORS",	_MIPS_CS_NUM_PROCESSORS},
#endif
#ifdef _MIPS_CS_OSREL_MAJ
    {"MIPS_CS_OSREL_MAJ",	_MIPS_CS_OSREL_MAJ},
#endif
#ifdef _MIPS_CS_OSREL_MIN
    {"MIPS_CS_OSREL_MIN",	_MIPS_CS_OSREL_MIN},
#endif
#ifdef _MIPS_CS_OSREL_PATCH
    {"MIPS_CS_OSREL_PATCH",	_MIPS_CS_OSREL_PATCH},
#endif
#ifdef _MIPS_CS_OS_NAME
    {"MIPS_CS_OS_NAME",	_MIPS_CS_OS_NAME},
#endif
#ifdef _MIPS_CS_OS_PROVIDER
    {"MIPS_CS_OS_PROVIDER",	_MIPS_CS_OS_PROVIDER},
#endif
#ifdef _MIPS_CS_PROCESSORS
    {"MIPS_CS_PROCESSORS",	_MIPS_CS_PROCESSORS},
#endif
#ifdef _MIPS_CS_SERIAL
    {"MIPS_CS_SERIAL",	_MIPS_CS_SERIAL},
#endif
#ifdef _MIPS_CS_VENDOR
    {"MIPS_CS_VENDOR",	_MIPS_CS_VENDOR},
#endif
};

static int
conv_confstr_confname(PyObject *arg, int *valuep)
{
    return conv_confname(arg, valuep, posix_constants_confstr,
                         sizeof(posix_constants_confstr)
                           / sizeof(struct constdef));
}

PyDoc_STRVAR(posix_confstr__doc__,
"confstr(name) -> string\n\n\
Return a string-valued system configuration variable.");

static PyObject *
posix_confstr(PyObject *self, PyObject *args)
{
    PyObject *result = NULL;
    int name;
    char buffer[64];

    if (PyArg_ParseTuple(args, "O&:confstr", conv_confstr_confname, &name)) {
        int len = confstr(name, buffer, sizeof(buffer));

        errno = 0;
        if (len == 0) {
            if (errno != 0)
                posix_error();
            else
                result = PyString_FromString("");
        }
        else {
            if (len >= sizeof(buffer)) {
                result = PyString_FromStringAndSize(NULL, len);
                if (result != NULL)
                    confstr(name, PyString_AS_STRING(result), len+1);
            }
            else
                result = PyString_FromString(buffer);
        }
    }
    return result;
}
#endif


#ifdef HAVE_SYSCONF
static struct constdef posix_constants_sysconf[] = {
#ifdef _SC_2_CHAR_TERM
    {"SC_2_CHAR_TERM",	_SC_2_CHAR_TERM},
#endif
#ifdef _SC_2_C_BIND
    {"SC_2_C_BIND",	_SC_2_C_BIND},
#endif
#ifdef _SC_2_C_DEV
    {"SC_2_C_DEV",	_SC_2_C_DEV},
#endif
#ifdef _SC_2_C_VERSION
    {"SC_2_C_VERSION",	_SC_2_C_VERSION},
#endif
#ifdef _SC_2_FORT_DEV
    {"SC_2_FORT_DEV",	_SC_2_FORT_DEV},
#endif
#ifdef _SC_2_FORT_RUN
    {"SC_2_FORT_RUN",	_SC_2_FORT_RUN},
#endif
#ifdef _SC_2_LOCALEDEF
    {"SC_2_LOCALEDEF",	_SC_2_LOCALEDEF},
#endif
#ifdef _SC_2_SW_DEV
    {"SC_2_SW_DEV",	_SC_2_SW_DEV},
#endif
#ifdef _SC_2_UPE
    {"SC_2_UPE",	_SC_2_UPE},
#endif
#ifdef _SC_2_VERSION
    {"SC_2_VERSION",	_SC_2_VERSION},
#endif
#ifdef _SC_ABI_ASYNCHRONOUS_IO
    {"SC_ABI_ASYNCHRONOUS_IO",	_SC_ABI_ASYNCHRONOUS_IO},
#endif
#ifdef _SC_ACL
    {"SC_ACL",	_SC_ACL},
#endif
#ifdef _SC_AIO_LISTIO_MAX
    {"SC_AIO_LISTIO_MAX",	_SC_AIO_LISTIO_MAX},
#endif
#ifdef _SC_AIO_MAX
    {"SC_AIO_MAX",	_SC_AIO_MAX},
#endif
#ifdef _SC_AIO_PRIO_DELTA_MAX
    {"SC_AIO_PRIO_DELTA_MAX",	_SC_AIO_PRIO_DELTA_MAX},
#endif
#ifdef _SC_ARG_MAX
    {"SC_ARG_MAX",	_SC_ARG_MAX},
#endif
#ifdef _SC_ASYNCHRONOUS_IO
    {"SC_ASYNCHRONOUS_IO",	_SC_ASYNCHRONOUS_IO},
#endif
#ifdef _SC_ATEXIT_MAX
    {"SC_ATEXIT_MAX",	_SC_ATEXIT_MAX},
#endif
#ifdef _SC_AUDIT
    {"SC_AUDIT",	_SC_AUDIT},
#endif
#ifdef _SC_AVPHYS_PAGES
    {"SC_AVPHYS_PAGES",	_SC_AVPHYS_PAGES},
#endif
#ifdef _SC_BC_BASE_MAX
    {"SC_BC_BASE_MAX",	_SC_BC_BASE_MAX},
#endif
#ifdef _SC_BC_DIM_MAX
    {"SC_BC_DIM_MAX",	_SC_BC_DIM_MAX},
#endif
#ifdef _SC_BC_SCALE_MAX
    {"SC_BC_SCALE_MAX",	_SC_BC_SCALE_MAX},
#endif
#ifdef _SC_BC_STRING_MAX
    {"SC_BC_STRING_MAX",	_SC_BC_STRING_MAX},
#endif
#ifdef _SC_CAP
    {"SC_CAP",	_SC_CAP},
#endif
#ifdef _SC_CHARCLASS_NAME_MAX
    {"SC_CHARCLASS_NAME_MAX",	_SC_CHARCLASS_NAME_MAX},
#endif
#ifdef _SC_CHAR_BIT
    {"SC_CHAR_BIT",	_SC_CHAR_BIT},
#endif
#ifdef _SC_CHAR_MAX
    {"SC_CHAR_MAX",	_SC_CHAR_MAX},
#endif
#ifdef _SC_CHAR_MIN
    {"SC_CHAR_MIN",	_SC_CHAR_MIN},
#endif
#ifdef _SC_CHILD_MAX
    {"SC_CHILD_MAX",	_SC_CHILD_MAX},
#endif
#ifdef _SC_CLK_TCK
    {"SC_CLK_TCK",	_SC_CLK_TCK},
#endif
#ifdef _SC_COHER_BLKSZ
    {"SC_COHER_BLKSZ",	_SC_COHER_BLKSZ},
#endif
#ifdef _SC_COLL_WEIGHTS_MAX
    {"SC_COLL_WEIGHTS_MAX",	_SC_COLL_WEIGHTS_MAX},
#endif
#ifdef _SC_DCACHE_ASSOC
    {"SC_DCACHE_ASSOC",	_SC_DCACHE_ASSOC},
#endif
#ifdef _SC_DCACHE_BLKSZ
    {"SC_DCACHE_BLKSZ",	_SC_DCACHE_BLKSZ},
#endif
#ifdef _SC_DCACHE_LINESZ
    {"SC_DCACHE_LINESZ",	_SC_DCACHE_LINESZ},
#endif
#ifdef _SC_DCACHE_SZ
    {"SC_DCACHE_SZ",	_SC_DCACHE_SZ},
#endif
#ifdef _SC_DCACHE_TBLKSZ
    {"SC_DCACHE_TBLKSZ",	_SC_DCACHE_TBLKSZ},
#endif
#ifdef _SC_DELAYTIMER_MAX
    {"SC_DELAYTIMER_MAX",	_SC_DELAYTIMER_MAX},
#endif
#ifdef _SC_EQUIV_CLASS_MAX
    {"SC_EQUIV_CLASS_MAX",	_SC_EQUIV_CLASS_MAX},
#endif
#ifdef _SC_EXPR_NEST_MAX
    {"SC_EXPR_NEST_MAX",	_SC_EXPR_NEST_MAX},
#endif
#ifdef _SC_FSYNC
    {"SC_FSYNC",	_SC_FSYNC},
#endif
#ifdef _SC_GETGR_R_SIZE_MAX
    {"SC_GETGR_R_SIZE_MAX",	_SC_GETGR_R_SIZE_MAX},
#endif
#ifdef _SC_GETPW_R_SIZE_MAX
    {"SC_GETPW_R_SIZE_MAX",	_SC_GETPW_R_SIZE_MAX},
#endif
#ifdef _SC_ICACHE_ASSOC
    {"SC_ICACHE_ASSOC",	_SC_ICACHE_ASSOC},
#endif
#ifdef _SC_ICACHE_BLKSZ
    {"SC_ICACHE_BLKSZ",	_SC_ICACHE_BLKSZ},
#endif
#ifdef _SC_ICACHE_LINESZ
    {"SC_ICACHE_LINESZ",	_SC_ICACHE_LINESZ},
#endif
#ifdef _SC_ICACHE_SZ
    {"SC_ICACHE_SZ",	_SC_ICACHE_SZ},
#endif
#ifdef _SC_INF
    {"SC_INF",	_SC_INF},
#endif
#ifdef _SC_INT_MAX
    {"SC_INT_MAX",	_SC_INT_MAX},
#endif
#ifdef _SC_INT_MIN
    {"SC_INT_MIN",	_SC_INT_MIN},
#endif
#ifdef _SC_IOV_MAX
    {"SC_IOV_MAX",	_SC_IOV_MAX},
#endif
#ifdef _SC_IP_SECOPTS
    {"SC_IP_SECOPTS",	_SC_IP_SECOPTS},
#endif
#ifdef _SC_JOB_CONTROL
    {"SC_JOB_CONTROL",	_SC_JOB_CONTROL},
#endif
#ifdef _SC_KERN_POINTERS
    {"SC_KERN_POINTERS",	_SC_KERN_POINTERS},
#endif
#ifdef _SC_KERN_SIM
    {"SC_KERN_SIM",	_SC_KERN_SIM},
#endif
#ifdef _SC_LINE_MAX
    {"SC_LINE_MAX",	_SC_LINE_MAX},
#endif
#ifdef _SC_LOGIN_NAME_MAX
    {"SC_LOGIN_NAME_MAX",	_SC_LOGIN_NAME_MAX},
#endif
#ifdef _SC_LOGNAME_MAX
    {"SC_LOGNAME_MAX",	_SC_LOGNAME_MAX},
#endif
#ifdef _SC_LONG_BIT
    {"SC_LONG_BIT",	_SC_LONG_BIT},
#endif
#ifdef _SC_MAC
    {"SC_MAC",	_SC_MAC},
#endif
#ifdef _SC_MAPPED_FILES
    {"SC_MAPPED_FILES",	_SC_MAPPED_FILES},
#endif
#ifdef _SC_MAXPID
    {"SC_MAXPID",	_SC_MAXPID},
#endif
#ifdef _SC_MB_LEN_MAX
    {"SC_MB_LEN_MAX",	_SC_MB_LEN_MAX},
#endif
#ifdef _SC_MEMLOCK
    {"SC_MEMLOCK",	_SC_MEMLOCK},
#endif
#ifdef _SC_MEMLOCK_RANGE
    {"SC_MEMLOCK_RANGE",	_SC_MEMLOCK_RANGE},
#endif
#ifdef _SC_MEMORY_PROTECTION
    {"SC_MEMORY_PROTECTION",	_SC_MEMORY_PROTECTION},
#endif
#ifdef _SC_MESSAGE_PASSING
    {"SC_MESSAGE_PASSING",	_SC_MESSAGE_PASSING},
#endif
#ifdef _SC_MMAP_FIXED_ALIGNMENT
    {"SC_MMAP_FIXED_ALIGNMENT",	_SC_MMAP_FIXED_ALIGNMENT},
#endif
#ifdef _SC_MQ_OPEN_MAX
    {"SC_MQ_OPEN_MAX",	_SC_MQ_OPEN_MAX},
#endif
#ifdef _SC_MQ_PRIO_MAX
    {"SC_MQ_PRIO_MAX",	_SC_MQ_PRIO_MAX},
#endif
#ifdef _SC_NACLS_MAX
    {"SC_NACLS_MAX",	_SC_NACLS_MAX},
#endif
#ifdef _SC_NGROUPS_MAX
    {"SC_NGROUPS_MAX",	_SC_NGROUPS_MAX},
#endif
#ifdef _SC_NL_ARGMAX
    {"SC_NL_ARGMAX",	_SC_NL_ARGMAX},
#endif
#ifdef _SC_NL_LANGMAX
    {"SC_NL_LANGMAX",	_SC_NL_LANGMAX},
#endif
#ifdef _SC_NL_MSGMAX
    {"SC_NL_MSGMAX",	_SC_NL_MSGMAX},
#endif
#ifdef _SC_NL_NMAX
    {"SC_NL_NMAX",	_SC_NL_NMAX},
#endif
#ifdef _SC_NL_SETMAX
    {"SC_NL_SETMAX",	_SC_NL_SETMAX},
#endif
#ifdef _SC_NL_TEXTMAX
    {"SC_NL_TEXTMAX",	_SC_NL_TEXTMAX},
#endif
#ifdef _SC_NPROCESSORS_CONF
    {"SC_NPROCESSORS_CONF",	_SC_NPROCESSORS_CONF},
#endif
#ifdef _SC_NPROCESSORS_ONLN
    {"SC_NPROCESSORS_ONLN",	_SC_NPROCESSORS_ONLN},
#endif
#ifdef _SC_NPROC_CONF
    {"SC_NPROC_CONF",	_SC_NPROC_CONF},
#endif
#ifdef _SC_NPROC_ONLN
    {"SC_NPROC_ONLN",	_SC_NPROC_ONLN},
#endif
#ifdef _SC_NZERO
    {"SC_NZERO",	_SC_NZERO},
#endif
#ifdef _SC_OPEN_MAX
    {"SC_OPEN_MAX",	_SC_OPEN_MAX},
#endif
#ifdef _SC_PAGESIZE
    {"SC_PAGESIZE",	_SC_PAGESIZE},
#endif
#ifdef _SC_PAGE_SIZE
    {"SC_PAGE_SIZE",	_SC_PAGE_SIZE},
#endif
#ifdef _SC_PASS_MAX
    {"SC_PASS_MAX",	_SC_PASS_MAX},
#endif
#ifdef _SC_PHYS_PAGES
    {"SC_PHYS_PAGES",	_SC_PHYS_PAGES},
#endif
#ifdef _SC_PII
    {"SC_PII",	_SC_PII},
#endif
#ifdef _SC_PII_INTERNET
    {"SC_PII_INTERNET",	_SC_PII_INTERNET},
#endif
#ifdef _SC_PII_INTERNET_DGRAM
    {"SC_PII_INTERNET_DGRAM",	_SC_PII_INTERNET_DGRAM},
#endif
#ifdef _SC_PII_INTERNET_STREAM
    {"SC_PII_INTERNET_STREAM",	_SC_PII_INTERNET_STREAM},
#endif
#ifdef _SC_PII_OSI
    {"SC_PII_OSI",	_SC_PII_OSI},
#endif
#ifdef _SC_PII_OSI_CLTS
    {"SC_PII_OSI_CLTS",	_SC_PII_OSI_CLTS},
#endif
#ifdef _SC_PII_OSI_COTS
    {"SC_PII_OSI_COTS",	_SC_PII_OSI_COTS},
#endif
#ifdef _SC_PII_OSI_M
    {"SC_PII_OSI_M",	_SC_PII_OSI_M},
#endif
#ifdef _SC_PII_SOCKET
    {"SC_PII_SOCKET",	_SC_PII_SOCKET},
#endif
#ifdef _SC_PII_XTI
    {"SC_PII_XTI",	_SC_PII_XTI},
#endif
#ifdef _SC_POLL
    {"SC_POLL",	_SC_POLL},
#endif
#ifdef _SC_PRIORITIZED_IO
    {"SC_PRIORITIZED_IO",	_SC_PRIORITIZED_IO},
#endif
#ifdef _SC_PRIORITY_SCHEDULING
    {"SC_PRIORITY_SCHEDULING",	_SC_PRIORITY_SCHEDULING},
#endif
#ifdef _SC_REALTIME_SIGNALS
    {"SC_REALTIME_SIGNALS",	_SC_REALTIME_SIGNALS},
#endif
#ifdef _SC_RE_DUP_MAX
    {"SC_RE_DUP_MAX",	_SC_RE_DUP_MAX},
#endif
#ifdef _SC_RTSIG_MAX
    {"SC_RTSIG_MAX",	_SC_RTSIG_MAX},
#endif
#ifdef _SC_SAVED_IDS
    {"SC_SAVED_IDS",	_SC_SAVED_IDS},
#endif
#ifdef _SC_SCHAR_MAX
    {"SC_SCHAR_MAX",	_SC_SCHAR_MAX},
#endif
#ifdef _SC_SCHAR_MIN
    {"SC_SCHAR_MIN",	_SC_SCHAR_MIN},
#endif
#ifdef _SC_SELECT
    {"SC_SELECT",	_SC_SELECT},
#endif
#ifdef _SC_SEMAPHORES
    {"SC_SEMAPHORES",	_SC_SEMAPHORES},
#endif
#ifdef _SC_SEM_NSEMS_MAX
    {"SC_SEM_NSEMS_MAX",	_SC_SEM_NSEMS_MAX},
#endif
#ifdef _SC_SEM_VALUE_MAX
    {"SC_SEM_VALUE_MAX",	_SC_SEM_VALUE_MAX},
#endif
#ifdef _SC_SHARED_MEMORY_OBJECTS
    {"SC_SHARED_MEMORY_OBJECTS",	_SC_SHARED_MEMORY_OBJECTS},
#endif
#ifdef _SC_SHRT_MAX
    {"SC_SHRT_MAX",	_SC_SHRT_MAX},
#endif
#ifdef _SC_SHRT_MIN
    {"SC_SHRT_MIN",	_SC_SHRT_MIN},
#endif
#ifdef _SC_SIGQUEUE_MAX
    {"SC_SIGQUEUE_MAX",	_SC_SIGQUEUE_MAX},
#endif
#ifdef _SC_SIGRT_MAX
    {"SC_SIGRT_MAX",	_SC_SIGRT_MAX},
#endif
#ifdef _SC_SIGRT_MIN
    {"SC_SIGRT_MIN",	_SC_SIGRT_MIN},
#endif
#ifdef _SC_SOFTPOWER
    {"SC_SOFTPOWER",	_SC_SOFTPOWER},
#endif
#ifdef _SC_SPLIT_CACHE
    {"SC_SPLIT_CACHE",	_SC_SPLIT_CACHE},
#endif
#ifdef _SC_SSIZE_MAX
    {"SC_SSIZE_MAX",	_SC_SSIZE_MAX},
#endif
#ifdef _SC_STACK_PROT
    {"SC_STACK_PROT",	_SC_STACK_PROT},
#endif
#ifdef _SC_STREAM_MAX
    {"SC_STREAM_MAX",	_SC_STREAM_MAX},
#endif
#ifdef _SC_SYNCHRONIZED_IO
    {"SC_SYNCHRONIZED_IO",	_SC_SYNCHRONIZED_IO},
#endif
#ifdef _SC_THREADS
    {"SC_THREADS",	_SC_THREADS},
#endif
#ifdef _SC_THREAD_ATTR_STACKADDR
    {"SC_THREAD_ATTR_STACKADDR",	_SC_THREAD_ATTR_STACKADDR},
#endif
#ifdef _SC_THREAD_ATTR_STACKSIZE
    {"SC_THREAD_ATTR_STACKSIZE",	_SC_THREAD_ATTR_STACKSIZE},
#endif
#ifdef _SC_THREAD_DESTRUCTOR_ITERATIONS
    {"SC_THREAD_DESTRUCTOR_ITERATIONS",	_SC_THREAD_DESTRUCTOR_ITERATIONS},
#endif
#ifdef _SC_THREAD_KEYS_MAX
    {"SC_THREAD_KEYS_MAX",	_SC_THREAD_KEYS_MAX},
#endif
#ifdef _SC_THREAD_PRIORITY_SCHEDULING
    {"SC_THREAD_PRIORITY_SCHEDULING",	_SC_THREAD_PRIORITY_SCHEDULING},
#endif
#ifdef _SC_THREAD_PRIO_INHERIT
    {"SC_THREAD_PRIO_INHERIT",	_SC_THREAD_PRIO_INHERIT},
#endif
#ifdef _SC_THREAD_PRIO_PROTECT
    {"SC_THREAD_PRIO_PROTECT",	_SC_THREAD_PRIO_PROTECT},
#endif
#ifdef _SC_THREAD_PROCESS_SHARED
    {"SC_THREAD_PROCESS_SHARED",	_SC_THREAD_PROCESS_SHARED},
#endif
#ifdef _SC_THREAD_SAFE_FUNCTIONS
    {"SC_THREAD_SAFE_FUNCTIONS",	_SC_THREAD_SAFE_FUNCTIONS},
#endif
#ifdef _SC_THREAD_STACK_MIN
    {"SC_THREAD_STACK_MIN",	_SC_THREAD_STACK_MIN},
#endif
#ifdef _SC_THREAD_THREADS_MAX
    {"SC_THREAD_THREADS_MAX",	_SC_THREAD_THREADS_MAX},
#endif
#ifdef _SC_TIMERS
    {"SC_TIMERS",	_SC_TIMERS},
#endif
#ifdef _SC_TIMER_MAX
    {"SC_TIMER_MAX",	_SC_TIMER_MAX},
#endif
#ifdef _SC_TTY_NAME_MAX
    {"SC_TTY_NAME_MAX",	_SC_TTY_NAME_MAX},
#endif
#ifdef _SC_TZNAME_MAX
    {"SC_TZNAME_MAX",	_SC_TZNAME_MAX},
#endif
#ifdef _SC_T_IOV_MAX
    {"SC_T_IOV_MAX",	_SC_T_IOV_MAX},
#endif
#ifdef _SC_UCHAR_MAX
    {"SC_UCHAR_MAX",	_SC_UCHAR_MAX},
#endif
#ifdef _SC_UINT_MAX
    {"SC_UINT_MAX",	_SC_UINT_MAX},
#endif
#ifdef _SC_UIO_MAXIOV
    {"SC_UIO_MAXIOV",	_SC_UIO_MAXIOV},
#endif
#ifdef _SC_ULONG_MAX
    {"SC_ULONG_MAX",	_SC_ULONG_MAX},
#endif
#ifdef _SC_USHRT_MAX
    {"SC_USHRT_MAX",	_SC_USHRT_MAX},
#endif
#ifdef _SC_VERSION
    {"SC_VERSION",	_SC_VERSION},
#endif
#ifdef _SC_WORD_BIT
    {"SC_WORD_BIT",	_SC_WORD_BIT},
#endif
#ifdef _SC_XBS5_ILP32_OFF32
    {"SC_XBS5_ILP32_OFF32",	_SC_XBS5_ILP32_OFF32},
#endif
#ifdef _SC_XBS5_ILP32_OFFBIG
    {"SC_XBS5_ILP32_OFFBIG",	_SC_XBS5_ILP32_OFFBIG},
#endif
#ifdef _SC_XBS5_LP64_OFF64
    {"SC_XBS5_LP64_OFF64",	_SC_XBS5_LP64_OFF64},
#endif
#ifdef _SC_XBS5_LPBIG_OFFBIG
    {"SC_XBS5_LPBIG_OFFBIG",	_SC_XBS5_LPBIG_OFFBIG},
#endif
#ifdef _SC_XOPEN_CRYPT
    {"SC_XOPEN_CRYPT",	_SC_XOPEN_CRYPT},
#endif
#ifdef _SC_XOPEN_ENH_I18N
    {"SC_XOPEN_ENH_I18N",	_SC_XOPEN_ENH_I18N},
#endif
#ifdef _SC_XOPEN_LEGACY
    {"SC_XOPEN_LEGACY",	_SC_XOPEN_LEGACY},
#endif
#ifdef _SC_XOPEN_REALTIME
    {"SC_XOPEN_REALTIME",	_SC_XOPEN_REALTIME},
#endif
#ifdef _SC_XOPEN_REALTIME_THREADS
    {"SC_XOPEN_REALTIME_THREADS",	_SC_XOPEN_REALTIME_THREADS},
#endif
#ifdef _SC_XOPEN_SHM
    {"SC_XOPEN_SHM",	_SC_XOPEN_SHM},
#endif
#ifdef _SC_XOPEN_UNIX
    {"SC_XOPEN_UNIX",	_SC_XOPEN_UNIX},
#endif
#ifdef _SC_XOPEN_VERSION
    {"SC_XOPEN_VERSION",	_SC_XOPEN_VERSION},
#endif
#ifdef _SC_XOPEN_XCU_VERSION
    {"SC_XOPEN_XCU_VERSION",	_SC_XOPEN_XCU_VERSION},
#endif
#ifdef _SC_XOPEN_XPG2
    {"SC_XOPEN_XPG2",	_SC_XOPEN_XPG2},
#endif
#ifdef _SC_XOPEN_XPG3
    {"SC_XOPEN_XPG3",	_SC_XOPEN_XPG3},
#endif
#ifdef _SC_XOPEN_XPG4
    {"SC_XOPEN_XPG4",	_SC_XOPEN_XPG4},
#endif
};

static int
conv_sysconf_confname(PyObject *arg, int *valuep)
{
    return conv_confname(arg, valuep, posix_constants_sysconf,
                         sizeof(posix_constants_sysconf)
                           / sizeof(struct constdef));
}

PyDoc_STRVAR(posix_sysconf__doc__,
"sysconf(name) -> integer\n\n\
Return an integer-valued system configuration variable.");

static PyObject *
posix_sysconf(PyObject *self, PyObject *args)
{
    PyObject *result = NULL;
    int name;

    if (PyArg_ParseTuple(args, "O&:sysconf", conv_sysconf_confname, &name)) {
        int value;

        errno = 0;
        value = sysconf(name);
        if (value == -1 && errno != 0)
            posix_error();
        else
            result = PyInt_FromLong(value);
    }
    return result;
}
#endif


/* This code is used to ensure that the tables of configuration value names
 * are in sorted order as required by conv_confname(), and also to build the
 * the exported dictionaries that are used to publish information about the
 * names available on the host platform.
 *
 * Sorting the table at runtime ensures that the table is properly ordered
 * when used, even for platforms we're not able to test on.  It also makes
 * it easier to add additional entries to the tables.
 */

static int
cmp_constdefs(const void *v1,  const void *v2)
{
    const struct constdef *c1 =
        (const struct constdef *) v1;
    const struct constdef *c2 =
        (const struct constdef *) v2;

    return strcmp(c1->name, c2->name);
}

static int
setup_confname_table(struct constdef *table, size_t tablesize,
		     char *tablename, PyObject *module)
{
    PyObject *d = NULL;
    size_t i;

    qsort(table, tablesize, sizeof(struct constdef), cmp_constdefs);
    d = PyDict_New();
    if (d == NULL)
	    return -1;

    for (i=0; i < tablesize; ++i) {
            PyObject *o = PyInt_FromLong(table[i].value);
            if (o == NULL || PyDict_SetItemString(d, table[i].name, o) == -1) {
		    Py_XDECREF(o);
		    Py_DECREF(d);
		    return -1;
            }
	    Py_DECREF(o);
    }
    return PyModule_AddObject(module, tablename, d);
}

/* Return -1 on failure, 0 on success. */
static int
setup_confname_tables(PyObject *module)
{
#if defined(HAVE_FPATHCONF) || defined(HAVE_PATHCONF)
    if (setup_confname_table(posix_constants_pathconf,
                             sizeof(posix_constants_pathconf)
                               / sizeof(struct constdef),
                             "pathconf_names", module))
        return -1;
#endif
#ifdef HAVE_CONFSTR
    if (setup_confname_table(posix_constants_confstr,
                             sizeof(posix_constants_confstr)
                               / sizeof(struct constdef),
                             "confstr_names", module))
        return -1;
#endif
#ifdef HAVE_SYSCONF
    if (setup_confname_table(posix_constants_sysconf,
                             sizeof(posix_constants_sysconf)
                               / sizeof(struct constdef),
                             "sysconf_names", module))
        return -1;
#endif
    return 0;
}


PyDoc_STRVAR(posix_abort__doc__,
"abort() -> does not return!\n\n\
Abort the interpreter immediately.  This 'dumps core' or otherwise fails\n\
in the hardest way possible on the hosting operating system.");

static PyObject *
posix_abort(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":abort"))
        return NULL;
    abort();
    /*NOTREACHED*/
    Py_FatalError("abort() called from Python code didn't abort!");
    return NULL;
}

#ifdef MS_WINDOWS
PyDoc_STRVAR(win32_startfile__doc__,
"startfile(filepath) - Start a file with its associated application.\n\
\n\
This acts like double-clicking the file in Explorer, or giving the file\n\
name as an argument to the DOS \"start\" command:  the file is opened\n\
with whatever application (if any) its extension is associated.\n\
\n\
startfile returns as soon as the associated application is launched.\n\
There is no option to wait for the application to close, and no way\n\
to retrieve the application's exit status.\n\
\n\
The filepath is relative to the current directory.  If you want to use\n\
an absolute path, make sure the first character is not a slash (\"/\");\n\
the underlying Win32 ShellExecute function doesn't work if it is.");

static PyObject *
win32_startfile(PyObject *self, PyObject *args)
{
	char *filepath;
	HINSTANCE rc;
	if (!PyArg_ParseTuple(args, "s:startfile", &filepath))
		return NULL;
	Py_BEGIN_ALLOW_THREADS
	rc = ShellExecute((HWND)0, NULL, filepath, NULL, NULL, SW_SHOWNORMAL);
	Py_END_ALLOW_THREADS
	if (rc <= (HINSTANCE)32)
		return win32_error("startfile", filepath);
	Py_INCREF(Py_None);
	return Py_None;
}
#endif

static PyMethodDef posix_methods[] = {
	{"access",	posix_access, METH_VARARGS, posix_access__doc__},
#ifdef HAVE_TTYNAME
	{"ttyname",	posix_ttyname, METH_VARARGS, posix_ttyname__doc__},
#endif
	{"chdir",	posix_chdir, METH_VARARGS, posix_chdir__doc__},
	{"chmod",	posix_chmod, METH_VARARGS, posix_chmod__doc__},
#ifdef HAVE_CHOWN
	{"chown",	posix_chown, METH_VARARGS, posix_chown__doc__},
#endif /* HAVE_CHOWN */
#ifdef HAVE_LCHOWN
	{"lchown",	posix_lchown, METH_VARARGS, posix_lchown__doc__},
#endif /* HAVE_LCHOWN */
#ifdef HAVE_CHROOT
	{"chroot",	posix_chroot, METH_VARARGS, posix_chroot__doc__},
#endif
#ifdef HAVE_CTERMID
	{"ctermid",	posix_ctermid, METH_VARARGS, posix_ctermid__doc__},
#endif
#ifdef HAVE_GETCWD
	{"getcwd",	posix_getcwd, METH_VARARGS, posix_getcwd__doc__},
	{"getcwdu",	posix_getcwdu, METH_VARARGS, posix_getcwdu__doc__},
#endif
#ifdef HAVE_LINK
	{"link",	posix_link, METH_VARARGS, posix_link__doc__},
#endif /* HAVE_LINK */
	{"listdir",	posix_listdir, METH_VARARGS, posix_listdir__doc__},
	{"lstat",	posix_lstat, METH_VARARGS, posix_lstat__doc__},
	{"mkdir",	posix_mkdir, METH_VARARGS, posix_mkdir__doc__},
#ifdef HAVE_NICE
	{"nice",	posix_nice, METH_VARARGS, posix_nice__doc__},
#endif /* HAVE_NICE */
#ifdef HAVE_READLINK
	{"readlink",	posix_readlink, METH_VARARGS, posix_readlink__doc__},
#endif /* HAVE_READLINK */
	{"rename",	posix_rename, METH_VARARGS, posix_rename__doc__},
	{"rmdir",	posix_rmdir, METH_VARARGS, posix_rmdir__doc__},
	{"stat",	posix_stat, METH_VARARGS, posix_stat__doc__},
#ifdef HAVE_SYMLINK
	{"symlink",	posix_symlink, METH_VARARGS, posix_symlink__doc__},
#endif /* HAVE_SYMLINK */
#ifdef HAVE_SYSTEM
	{"system",	posix_system, METH_VARARGS, posix_system__doc__},
#endif
	{"umask",	posix_umask, METH_VARARGS, posix_umask__doc__},
#ifdef HAVE_UNAME
	{"uname",	posix_uname, METH_VARARGS, posix_uname__doc__},
#endif /* HAVE_UNAME */
	{"unlink",	posix_unlink, METH_VARARGS, posix_unlink__doc__},
	{"remove",	posix_unlink, METH_VARARGS, posix_remove__doc__},
	{"utime",	posix_utime, METH_VARARGS, posix_utime__doc__},
#ifdef HAVE_TIMES
	{"times",	posix_times, METH_VARARGS, posix_times__doc__},
#endif /* HAVE_TIMES */
	{"_exit",	posix__exit, METH_VARARGS, posix__exit__doc__},
#ifdef HAVE_EXECV
	{"execv",	posix_execv, METH_VARARGS, posix_execv__doc__},
	{"execve",	posix_execve, METH_VARARGS, posix_execve__doc__},
#endif /* HAVE_EXECV */
#ifdef HAVE_SPAWNV
	{"spawnv",	posix_spawnv, METH_VARARGS, posix_spawnv__doc__},
	{"spawnve",	posix_spawnve, METH_VARARGS, posix_spawnve__doc__},
#endif /* HAVE_SPAWNV */
#ifdef HAVE_FORK1
	{"fork1",       posix_fork1, METH_VARARGS, posix_fork1__doc__},
#endif /* HAVE_FORK1 */
#ifdef HAVE_FORK
	{"fork",	posix_fork, METH_VARARGS, posix_fork__doc__},
#endif /* HAVE_FORK */
#if defined(HAVE_OPENPTY) || defined(HAVE__GETPTY)
	{"openpty",	posix_openpty, METH_VARARGS, posix_openpty__doc__},
#endif /* HAVE_OPENPTY || HAVE__GETPTY */
#ifdef HAVE_FORKPTY
	{"forkpty",	posix_forkpty, METH_VARARGS, posix_forkpty__doc__},
#endif /* HAVE_FORKPTY */
#ifdef HAVE_GETEGID
	{"getegid",	posix_getegid, METH_VARARGS, posix_getegid__doc__},
#endif /* HAVE_GETEGID */
#ifdef HAVE_GETEUID
	{"geteuid",	posix_geteuid, METH_VARARGS, posix_geteuid__doc__},
#endif /* HAVE_GETEUID */
#ifdef HAVE_GETGID
	{"getgid",	posix_getgid, METH_VARARGS, posix_getgid__doc__},
#endif /* HAVE_GETGID */
#ifdef HAVE_GETGROUPS
	{"getgroups",	posix_getgroups, METH_VARARGS, posix_getgroups__doc__},
#endif
	{"getpid",	posix_getpid, METH_VARARGS, posix_getpid__doc__},
#ifdef HAVE_GETPGRP
	{"getpgrp",	posix_getpgrp, METH_VARARGS, posix_getpgrp__doc__},
#endif /* HAVE_GETPGRP */
#ifdef HAVE_GETPPID
	{"getppid",	posix_getppid, METH_VARARGS, posix_getppid__doc__},
#endif /* HAVE_GETPPID */
#ifdef HAVE_GETUID
	{"getuid",	posix_getuid, METH_VARARGS, posix_getuid__doc__},
#endif /* HAVE_GETUID */
#ifdef HAVE_GETLOGIN
	{"getlogin",	posix_getlogin, METH_VARARGS, posix_getlogin__doc__},
#endif
#ifdef HAVE_KILL
	{"kill",	posix_kill, METH_VARARGS, posix_kill__doc__},
#endif /* HAVE_KILL */
#ifdef HAVE_KILLPG
	{"killpg",	posix_killpg, METH_VARARGS, posix_killpg__doc__},
#endif /* HAVE_KILLPG */
#ifdef HAVE_PLOCK
	{"plock",	posix_plock, METH_VARARGS, posix_plock__doc__},
#endif /* HAVE_PLOCK */
#ifdef HAVE_POPEN
	{"popen",	posix_popen, METH_VARARGS, posix_popen__doc__},
#ifdef MS_WINDOWS
	{"popen2",	win32_popen2, METH_VARARGS},
	{"popen3",	win32_popen3, METH_VARARGS},
	{"popen4",	win32_popen4, METH_VARARGS},
	{"startfile",	win32_startfile, METH_VARARGS, win32_startfile__doc__},
#else
#if defined(PYOS_OS2) && defined(PYCC_GCC)
	{"popen2",	os2emx_popen2, METH_VARARGS},
	{"popen3",	os2emx_popen3, METH_VARARGS},
	{"popen4",	os2emx_popen4, METH_VARARGS},
#endif
#endif
#endif /* HAVE_POPEN */
#ifdef HAVE_SETUID
	{"setuid",	posix_setuid, METH_VARARGS, posix_setuid__doc__},
#endif /* HAVE_SETUID */
#ifdef HAVE_SETEUID
	{"seteuid",	posix_seteuid, METH_VARARGS, posix_seteuid__doc__},
#endif /* HAVE_SETEUID */
#ifdef HAVE_SETEGID
	{"setegid",	posix_setegid, METH_VARARGS, posix_setegid__doc__},
#endif /* HAVE_SETEGID */
#ifdef HAVE_SETREUID
	{"setreuid",	posix_setreuid, METH_VARARGS, posix_setreuid__doc__},
#endif /* HAVE_SETREUID */
#ifdef HAVE_SETREGID
	{"setregid",	posix_setregid,	METH_VARARGS, posix_setregid__doc__},
#endif /* HAVE_SETREGID */
#ifdef HAVE_SETGID
	{"setgid",	posix_setgid, METH_VARARGS, posix_setgid__doc__},
#endif /* HAVE_SETGID */
#ifdef HAVE_SETGROUPS
	{"setgroups",	posix_setgroups, METH_VARARGS, posix_setgroups__doc__},
#endif /* HAVE_SETGROUPS */
#ifdef HAVE_GETPGID
	{"getpgid",	posix_getpgid, METH_VARARGS, posix_getpgid__doc__},
#endif /* HAVE_GETPGID */
#ifdef HAVE_SETPGRP
	{"setpgrp",	posix_setpgrp, METH_VARARGS, posix_setpgrp__doc__},
#endif /* HAVE_SETPGRP */
#ifdef HAVE_WAIT
	{"wait",	posix_wait, METH_VARARGS, posix_wait__doc__},
#endif /* HAVE_WAIT */
#if defined(HAVE_WAITPID) || defined(HAVE_CWAIT)
	{"waitpid",	posix_waitpid, METH_VARARGS, posix_waitpid__doc__},
#endif /* HAVE_WAITPID */
#ifdef HAVE_SETSID
	{"setsid",	posix_setsid, METH_VARARGS, posix_setsid__doc__},
#endif /* HAVE_SETSID */
#ifdef HAVE_SETPGID
	{"setpgid",	posix_setpgid, METH_VARARGS, posix_setpgid__doc__},
#endif /* HAVE_SETPGID */
#ifdef HAVE_TCGETPGRP
	{"tcgetpgrp",	posix_tcgetpgrp, METH_VARARGS, posix_tcgetpgrp__doc__},
#endif /* HAVE_TCGETPGRP */
#ifdef HAVE_TCSETPGRP
	{"tcsetpgrp",	posix_tcsetpgrp, METH_VARARGS, posix_tcsetpgrp__doc__},
#endif /* HAVE_TCSETPGRP */
	{"open",	posix_open, METH_VARARGS, posix_open__doc__},
	{"close",	posix_close, METH_VARARGS, posix_close__doc__},
	{"dup",		posix_dup, METH_VARARGS, posix_dup__doc__},
	{"dup2",	posix_dup2, METH_VARARGS, posix_dup2__doc__},
	{"lseek",	posix_lseek, METH_VARARGS, posix_lseek__doc__},
	{"read",	posix_read, METH_VARARGS, posix_read__doc__},
	{"write",	posix_write, METH_VARARGS, posix_write__doc__},
	{"fstat",	posix_fstat, METH_VARARGS, posix_fstat__doc__},
	{"fdopen",	posix_fdopen, METH_VARARGS, posix_fdopen__doc__},
	{"isatty",	posix_isatty, METH_VARARGS, posix_isatty__doc__},
#ifdef HAVE_PIPE
	{"pipe",	posix_pipe, METH_VARARGS, posix_pipe__doc__},
#endif
#ifdef HAVE_MKFIFO
	{"mkfifo",	posix_mkfifo, METH_VARARGS, posix_mkfifo__doc__},
#endif
#if defined(HAVE_MKNOD) && defined(HAVE_MAKEDEV)
	{"mknod",	posix_mknod, METH_VARARGS, posix_mknod__doc__},
#endif
#ifdef HAVE_FTRUNCATE
	{"ftruncate",	posix_ftruncate, METH_VARARGS, posix_ftruncate__doc__},
#endif
#ifdef HAVE_PUTENV
	{"putenv",	posix_putenv, METH_VARARGS, posix_putenv__doc__},
#endif
#ifdef HAVE_UNSETENV
	{"unsetenv",	posix_unsetenv, METH_VARARGS, posix_unsetenv__doc__},
#endif
#ifdef HAVE_STRERROR
	{"strerror",	posix_strerror, METH_VARARGS, posix_strerror__doc__},
#endif
#ifdef HAVE_FCHDIR
	{"fchdir",	posix_fchdir, METH_O, posix_fchdir__doc__},
#endif
#ifdef HAVE_FSYNC
	{"fsync",       posix_fsync, METH_O, posix_fsync__doc__},
#endif
#ifdef HAVE_FDATASYNC
	{"fdatasync",   posix_fdatasync,  METH_O, posix_fdatasync__doc__},
#endif
#ifdef HAVE_SYS_WAIT_H
#ifdef WCOREDUMP
        {"WCOREDUMP",	posix_WCOREDUMP, METH_VARARGS, posix_WCOREDUMP__doc__},
#endif /* WCOREDUMP */
#ifdef WIFCONTINUED
        {"WIFCONTINUED",posix_WIFCONTINUED, METH_VARARGS, posix_WIFCONTINUED__doc__},
#endif /* WIFCONTINUED */
#ifdef WIFSTOPPED
        {"WIFSTOPPED",	posix_WIFSTOPPED, METH_VARARGS, posix_WIFSTOPPED__doc__},
#endif /* WIFSTOPPED */
#ifdef WIFSIGNALED
        {"WIFSIGNALED",	posix_WIFSIGNALED, METH_VARARGS, posix_WIFSIGNALED__doc__},
#endif /* WIFSIGNALED */
#ifdef WIFEXITED
        {"WIFEXITED",	posix_WIFEXITED, METH_VARARGS, posix_WIFEXITED__doc__},
#endif /* WIFEXITED */
#ifdef WEXITSTATUS
        {"WEXITSTATUS",	posix_WEXITSTATUS, METH_VARARGS, posix_WEXITSTATUS__doc__},
#endif /* WEXITSTATUS */
#ifdef WTERMSIG
        {"WTERMSIG",	posix_WTERMSIG, METH_VARARGS, posix_WTERMSIG__doc__},
#endif /* WTERMSIG */
#ifdef WSTOPSIG
        {"WSTOPSIG",	posix_WSTOPSIG, METH_VARARGS, posix_WSTOPSIG__doc__},
#endif /* WSTOPSIG */
#endif /* HAVE_SYS_WAIT_H */
#ifdef HAVE_FSTATVFS
	{"fstatvfs",	posix_fstatvfs, METH_VARARGS, posix_fstatvfs__doc__},
#endif
#ifdef HAVE_STATVFS
	{"statvfs",	posix_statvfs, METH_VARARGS, posix_statvfs__doc__},
#endif
#ifdef HAVE_TMPFILE
	{"tmpfile",	posix_tmpfile, METH_VARARGS, posix_tmpfile__doc__},
#endif
#ifdef HAVE_TEMPNAM
	{"tempnam",	posix_tempnam, METH_VARARGS, posix_tempnam__doc__},
#endif
#ifdef HAVE_TMPNAM
	{"tmpnam",	posix_tmpnam, METH_VARARGS, posix_tmpnam__doc__},
#endif
#ifdef HAVE_CONFSTR
	{"confstr",	posix_confstr, METH_VARARGS, posix_confstr__doc__},
#endif
#ifdef HAVE_SYSCONF
	{"sysconf",	posix_sysconf, METH_VARARGS, posix_sysconf__doc__},
#endif
#ifdef HAVE_FPATHCONF
	{"fpathconf",	posix_fpathconf, METH_VARARGS, posix_fpathconf__doc__},
#endif
#ifdef HAVE_PATHCONF
	{"pathconf",	posix_pathconf, METH_VARARGS, posix_pathconf__doc__},
#endif
	{"abort",	posix_abort, METH_VARARGS, posix_abort__doc__},
#ifdef MS_WINDOWS
	{"_getfullpathname",	posix__getfullpathname, METH_VARARGS, NULL},
#endif
	{NULL,		NULL}		 /* Sentinel */
};


static int
ins(PyObject *module, char *symbol, long value)
{
        return PyModule_AddIntConstant(module, symbol, value);
}

#if defined(PYOS_OS2)
/* Insert Platform-Specific Constant Values (Strings & Numbers) of Common Use */
static int insertvalues(PyObject *module)
{
    APIRET    rc;
    ULONG     values[QSV_MAX+1];
    PyObject *v;
    char     *ver, tmp[50];

    Py_BEGIN_ALLOW_THREADS
    rc = DosQuerySysInfo(1, QSV_MAX, &values[1], sizeof(values));
    Py_END_ALLOW_THREADS

    if (rc != NO_ERROR) {
        os2_error(rc);
        return -1;
    }

    if (ins(module, "meminstalled", values[QSV_TOTPHYSMEM])) return -1;
    if (ins(module, "memkernel",    values[QSV_TOTRESMEM])) return -1;
    if (ins(module, "memvirtual",   values[QSV_TOTAVAILMEM])) return -1;
    if (ins(module, "maxpathlen",   values[QSV_MAX_PATH_LENGTH])) return -1;
    if (ins(module, "maxnamelen",   values[QSV_MAX_COMP_LENGTH])) return -1;
    if (ins(module, "revision",     values[QSV_VERSION_REVISION])) return -1;
    if (ins(module, "timeslice",    values[QSV_MIN_SLICE])) return -1;

    switch (values[QSV_VERSION_MINOR]) {
    case 0:  ver = "2.00"; break;
    case 10: ver = "2.10"; break;
    case 11: ver = "2.11"; break;
    case 30: ver = "3.00"; break;
    case 40: ver = "4.00"; break;
    case 50: ver = "5.00"; break;
    default:
        PyOS_snprintf(tmp, sizeof(tmp),
        	      "%d-%d", values[QSV_VERSION_MAJOR],
                      values[QSV_VERSION_MINOR]);
        ver = &tmp[0];
    }

    /* Add Indicator of the Version of the Operating System */
    if (PyModule_AddStringConstant(module, "version", tmp) < 0)
        return -1;

    /* Add Indicator of Which Drive was Used to Boot the System */
    tmp[0] = 'A' + values[QSV_BOOT_DRIVE] - 1;
    tmp[1] = ':';
    tmp[2] = '\0';

    return PyModule_AddStringConstant(module, "bootdrive", tmp);
}
#endif

static int
all_ins(PyObject *d)
{
#ifdef F_OK
        if (ins(d, "F_OK", (long)F_OK)) return -1;
#endif
#ifdef R_OK
        if (ins(d, "R_OK", (long)R_OK)) return -1;
#endif
#ifdef W_OK
        if (ins(d, "W_OK", (long)W_OK)) return -1;
#endif
#ifdef X_OK
        if (ins(d, "X_OK", (long)X_OK)) return -1;
#endif
#ifdef NGROUPS_MAX
        if (ins(d, "NGROUPS_MAX", (long)NGROUPS_MAX)) return -1;
#endif
#ifdef TMP_MAX
        if (ins(d, "TMP_MAX", (long)TMP_MAX)) return -1;
#endif
#ifdef WCONTINUED
        if (ins(d, "WCONTINUED", (long)WCONTINUED)) return -1;
#endif
#ifdef WNOHANG
        if (ins(d, "WNOHANG", (long)WNOHANG)) return -1;
#endif
#ifdef WUNTRACED
        if (ins(d, "WUNTRACED", (long)WUNTRACED)) return -1;
#endif
#ifdef O_RDONLY
        if (ins(d, "O_RDONLY", (long)O_RDONLY)) return -1;
#endif
#ifdef O_WRONLY
        if (ins(d, "O_WRONLY", (long)O_WRONLY)) return -1;
#endif
#ifdef O_RDWR
        if (ins(d, "O_RDWR", (long)O_RDWR)) return -1;
#endif
#ifdef O_NDELAY
        if (ins(d, "O_NDELAY", (long)O_NDELAY)) return -1;
#endif
#ifdef O_NONBLOCK
        if (ins(d, "O_NONBLOCK", (long)O_NONBLOCK)) return -1;
#endif
#ifdef O_APPEND
        if (ins(d, "O_APPEND", (long)O_APPEND)) return -1;
#endif
#ifdef O_DSYNC
        if (ins(d, "O_DSYNC", (long)O_DSYNC)) return -1;
#endif
#ifdef O_RSYNC
        if (ins(d, "O_RSYNC", (long)O_RSYNC)) return -1;
#endif
#ifdef O_SYNC
        if (ins(d, "O_SYNC", (long)O_SYNC)) return -1;
#endif
#ifdef O_NOCTTY
        if (ins(d, "O_NOCTTY", (long)O_NOCTTY)) return -1;
#endif
#ifdef O_CREAT
        if (ins(d, "O_CREAT", (long)O_CREAT)) return -1;
#endif
#ifdef O_EXCL
        if (ins(d, "O_EXCL", (long)O_EXCL)) return -1;
#endif
#ifdef O_TRUNC
        if (ins(d, "O_TRUNC", (long)O_TRUNC)) return -1;
#endif
#ifdef O_BINARY
        if (ins(d, "O_BINARY", (long)O_BINARY)) return -1;
#endif
#ifdef O_TEXT
        if (ins(d, "O_TEXT", (long)O_TEXT)) return -1;
#endif
#ifdef O_LARGEFILE
        if (ins(d, "O_LARGEFILE", (long)O_LARGEFILE)) return -1;
#endif

/* MS Windows */
#ifdef O_NOINHERIT
	/* Don't inherit in child processes. */
        if (ins(d, "O_NOINHERIT", (long)O_NOINHERIT)) return -1;
#endif
#ifdef _O_SHORT_LIVED
	/* Optimize for short life (keep in memory). */
	/* MS forgot to define this one with a non-underscore form too. */
        if (ins(d, "O_SHORT_LIVED", (long)_O_SHORT_LIVED)) return -1;
#endif
#ifdef O_TEMPORARY
	/* Automatically delete when last handle is closed. */
        if (ins(d, "O_TEMPORARY", (long)O_TEMPORARY)) return -1;
#endif
#ifdef O_RANDOM
	/* Optimize for random access. */
        if (ins(d, "O_RANDOM", (long)O_RANDOM)) return -1;
#endif
#ifdef O_SEQUENTIAL
	/* Optimize for sequential access. */
        if (ins(d, "O_SEQUENTIAL", (long)O_SEQUENTIAL)) return -1;
#endif

/* GNU extensions. */
#ifdef O_DIRECT
        /* Direct disk access. */
        if (ins(d, "O_DIRECT", (long)O_DIRECT)) return -1;
#endif
#ifdef O_DIRECTORY
        /* Must be a directory.	 */
        if (ins(d, "O_DIRECTORY", (long)O_DIRECTORY)) return -1;
#endif
#ifdef O_NOFOLLOW
        /* Do not follow links.	 */
        if (ins(d, "O_NOFOLLOW", (long)O_NOFOLLOW)) return -1;
#endif

#ifdef HAVE_SPAWNV
#if defined(PYOS_OS2) && defined(PYCC_GCC)
	if (ins(d, "P_WAIT", (long)P_WAIT)) return -1;
	if (ins(d, "P_NOWAIT", (long)P_NOWAIT)) return -1;
	if (ins(d, "P_OVERLAY", (long)P_OVERLAY)) return -1;
	if (ins(d, "P_DEBUG", (long)P_DEBUG)) return -1;
	if (ins(d, "P_SESSION", (long)P_SESSION)) return -1;
	if (ins(d, "P_DETACH", (long)P_DETACH)) return -1;
	if (ins(d, "P_PM", (long)P_PM)) return -1;
	if (ins(d, "P_DEFAULT", (long)P_DEFAULT)) return -1;
	if (ins(d, "P_MINIMIZE", (long)P_MINIMIZE)) return -1;
	if (ins(d, "P_MAXIMIZE", (long)P_MAXIMIZE)) return -1;
	if (ins(d, "P_FULLSCREEN", (long)P_FULLSCREEN)) return -1;
	if (ins(d, "P_WINDOWED", (long)P_WINDOWED)) return -1;
	if (ins(d, "P_FOREGROUND", (long)P_FOREGROUND)) return -1;
	if (ins(d, "P_BACKGROUND", (long)P_BACKGROUND)) return -1;
	if (ins(d, "P_NOCLOSE", (long)P_NOCLOSE)) return -1;
	if (ins(d, "P_NOSESSION", (long)P_NOSESSION)) return -1;
	if (ins(d, "P_QUOTE", (long)P_QUOTE)) return -1;
	if (ins(d, "P_TILDE", (long)P_TILDE)) return -1;
	if (ins(d, "P_UNRELATED", (long)P_UNRELATED)) return -1;
	if (ins(d, "P_DEBUGDESC", (long)P_DEBUGDESC)) return -1;
#else
        if (ins(d, "P_WAIT", (long)_P_WAIT)) return -1;
        if (ins(d, "P_NOWAIT", (long)_P_NOWAIT)) return -1;
        if (ins(d, "P_OVERLAY", (long)_OLD_P_OVERLAY)) return -1;
        if (ins(d, "P_NOWAITO", (long)_P_NOWAITO)) return -1;
        if (ins(d, "P_DETACH", (long)_P_DETACH)) return -1;
#endif
#endif

#if defined(PYOS_OS2)
        if (insertvalues(d)) return -1;
#endif
        return 0;
}


#if (defined(_MSC_VER) || defined(__WATCOMC__) || defined(__BORLANDC__)) && !defined(__QNX__)
#define INITFUNC initnt
#define MODNAME "nt"

#elif defined(PYOS_OS2)
#define INITFUNC initos2
#define MODNAME "os2"

#else
#define INITFUNC initposix
#define MODNAME "posix"
#endif

PyMODINIT_FUNC
INITFUNC(void)
{
	PyObject *m, *v;

	m = Py_InitModule3(MODNAME,
			   posix_methods,
			   posix__doc__);

	/* Initialize environ dictionary */
	v = convertenviron();
	Py_XINCREF(v);
	if (v == NULL || PyModule_AddObject(m, "environ", v) != 0)
		return;
	Py_DECREF(v);

        if (all_ins(m))
                return;

        if (setup_confname_tables(m))
                return;

	Py_INCREF(PyExc_OSError);
	PyModule_AddObject(m, "error", PyExc_OSError);

#ifdef HAVE_PUTENV
	if (posix_putenv_garbage == NULL)
		posix_putenv_garbage = PyDict_New();
#endif

	stat_result_desc.name = MODNAME ".stat_result";
	PyStructSequence_InitType(&StatResultType, &stat_result_desc);
	Py_INCREF((PyObject*) &StatResultType);
	PyModule_AddObject(m, "stat_result", (PyObject*) &StatResultType);

	statvfs_result_desc.name = MODNAME ".statvfs_result";
	PyStructSequence_InitType(&StatVFSResultType, &statvfs_result_desc);
	Py_INCREF((PyObject*) &StatVFSResultType);
	PyModule_AddObject(m, "statvfs_result",
			   (PyObject*) &StatVFSResultType);
}
