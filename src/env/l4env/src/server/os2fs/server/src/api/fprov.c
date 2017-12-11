/*  File provider proxy
 *  (gives a file as a dataspace
 *  by its OS/2-style pathname)
 *  -- is a frontend to another file provider
 *  or l4vfs.
 */

/* osFree internal */
#include <os3/MountReg.h>
#include <os3/globals.h>
#include <os3/io.h>

/* l4env includes */
#include <l4/dm_mem/dm_mem.h>
#include <l4/dm_generic/consts.h>

/* dice */
#include <dice/dice.h>

/* libc includes */
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

void DosNameConversion(char * pszName);

long DICE_CV
l4fprov_file_open_component (CORBA_Object _dice_corba_obj,
                      const char* fname /* in */,
                      const l4_threadid_t *dm /* in */,
                      unsigned long flags /* in */,
                      l4dm_dataspace_t *ds /* out */,
                      l4_size_t *size /* out */,
                      CORBA_Server_Environment *_dice_corba_env);

/* extract drive letter from path */
char get_drv(const char *s_path) {
    if(s_path[1] == ':')
        return s_path[0];
    return '\0';
}

/* extract directory from path */
char * get_directory(const char *s_path) 
{
  int s=0;
  int e=0;
  int i=0;

  if(s_path[1] == ':')
  {
    s=2;
  }

  for (i=s; i<=strlen(s_path); i++)
  {
    if (s_path[i]=='\\')
    {
      e=i;
    }
  }
  
  if (e==0) return '\0';
  char *dir=malloc(e-s+2);
  strncpy(dir, &s_path[s], e-s+1);
  dir[e - s + 1] = '\0';
  return dir;
}

/* extract name from path */
char * get_name(const char *s_path) 
{
  int s=0;
  int e=0;
  int i=0;

  for (i=0; i<=strlen(s_path); i++)
  {
    if (s_path[i]=='\\')
    {
      s=i+1;
    }
  }

  e=strlen(s_path);
  char *name=malloc(e-s+1);
  strncpy(name, &s_path[s], e-s+1);
  return name;
}

int
pathconv(char **converted, char *fname)
{
  struct I_Fs_srv *fsrv;
  char drv;
  char *directory;
  char *name;
  char *newfilename;
  char *newdirectory;

  drv = tolower(get_drv(fname));
  io_log("drv=%c:\n", drv);

  if(drv == '\0') 
  {
    return 1;
  }

  directory = get_directory(fname);
  io_log("directory=%s\n", directory);
  
  if (directory==NULL)
  {
    return 1;
  }
  
  name = get_name(fname);
  io_log("name=%s\n", name);

  DosNameConversion(directory);
  DosNameConversion(name);

  io_log("directory=%s\n", directory);
  io_log("name=%s\n", name);
  
  if (drv >= 'c' && drv <= 'z')
  { 
    fsrv = FSRouter_route(&fsrouter, drv);

    newdirectory=malloc(strlen(fsrv->mountpoint)+
                          strlen(directory)+1);

    strcpy(newdirectory, fsrv->mountpoint);
    newdirectory=strcat(newdirectory, directory);
  }
  else
  {
    newdirectory = malloc(strlen(directory) + 1);
    strcpy(newdirectory, directory);
  }

  newfilename=malloc(strlen(newdirectory)+
                     strlen(name)+1);

  strcpy(newfilename, newdirectory);
  newfilename=strcat(newfilename, name);

  io_log("newfilename=%s\n", newfilename);
  *converted = newfilename;

  return 0;
}

long DICE_CV
l4fprov_file_open_component (CORBA_Object _dice_corba_obj,
                      const char* fname /* in */,
                      const l4_threadid_t *dm /* in */,
                      unsigned long flags /* in */,
                      l4dm_dataspace_t *ds /* out */,
                      l4_size_t *size /* out */,
                      CORBA_Server_Environment *_dice_corba_env)
{
  struct stat stat;
  l4_addr_t addr;
  char *newfilename;
  int  handle;
  int  rc;

  /* convert OS/2 path to PN path */
  if (pathconv(&newfilename, (char *)fname))
   return 2; /* ERROR_FILE_NOT_FOUND */

  handle = open(newfilename, O_RDONLY);

  if(handle == -1)
      return 2; /* ERROR_FILE_NOT_FOUND */

  io_log("file opened\n");
  
  rc = fstat(handle, &stat);

  /* get length */
  *size = stat.st_size;

  /* Create a dataspace of a given size */
  rc = l4dm_mem_open(*dm, *size, 0, L4DM_RW, "fprov dataspace", ds);

  if (rc < 0)
    return 8; /* ERROR_NOT_ENOUGH_MEMORY */

  /* attach the created dataspace to our address space */
  rc = l4rm_attach(ds, *size, 0, L4DM_RW, (void **)&addr);

  if (rc < 0)
    return 8; /* What to return? */

  read(handle, (void *)addr, *size);

  close(handle);

  l4rm_detach((void *)addr);

  io_log("caller=%x.%x\n", _dice_corba_obj->id.task, _dice_corba_obj->id.lthread);  
  rc = l4dm_transfer(ds, *_dice_corba_obj);

  if (rc < 0)
  {
    io_log("error transferring ds\n");
    l4dm_close(ds);
    return 5; /* ERROR_ACCESS_DENIED */
  }

  io_log("successful return\n");

  return 0; /*NO_ERROR;*/
}
