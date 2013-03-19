#include "env-utils.h"
#include "xmalloc.h"

int env_buffer_init(env_buffer *self, const char * const* env_src)
{
	int result;
	if (self) {
		result = 0;
		self->size = 0;
		self->env = (char **)xmalloc(sizeof(char *) * 1);
		self->env[0] = NULL;
		if (env_src) {
			int i;
			for (i = 0; env_src[i]; i++) {
				result = env_buffer_add(env_src[i]);
				if (result) {
					break;
				}
			}
		}
		if (result) {
			env_buffer_free_env(self->env);
			self->env = NULL;
			self->size = 0;
		}
	}
	else {
		result = -1;
	}
	return result;
}

int env_buffer_close(env_buffer *self)
{
	if (self) {
		if (self->env) {
			env_buffer_free_env(self->env);
		}
	}
}
int env_buffer_add(env_buffer *self, char *entry)
{
	int result;
	if (self) {
		if (entry) {
			char **new_env;
			new_env = (char **)xmalloc
				(sizeof(char *) * self->size + 2);
			{
				int i;
				for (i = 0; i < self->size; i++) {
					new_env[i] = xstrdup(self->env[i]);
				}
				new_env[self->size] = xstrdup(entry);
				new_env[self->size + 1] = NULL;
				env_buffer_free_env(self->env);
				self->env = new_env;
			}
			result = 0;
		}
		else {
			result = 0;
		}
	}
	else {
		result = -1;
	}
	return result;
}
char **env_buffer_copy_env(env_buffer *self)
{
	char **result;
	if (self) {
		result = (char **)xmalloc
				(sizeof(char *) * self->size + 0);
		{
			int i;
			for (i = 0; i < self->size; i++) {
				result[i] = xstrdup(self->env[i]);
			}
			result[self->size + 1] = NULL;
		}
	}
	else {
		result = NULL;
	}
	return result;
}

void env_buffer_free_env(char **env)
{

	if (env)
	{
		int i;
		for (i = 0; env[i]; i++)
		{
			free(env[i]);
		}
		free(env);
	}
}


