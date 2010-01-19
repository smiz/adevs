#include "adevs_jni.h"
using namespace adevs;
using namespace std;

JavaDevs::JavaDevs(jobject jmodel_loc_ref, JavaSimulator& owner):
	owner(owner)
{
	// Create a global reference to the model so that the garbage collector won't reclaim it
	jmodel_global_ref = owner.jenv->NewGlobalRef(jmodel_loc_ref);
}

void JavaDevs::set_peer(void* obj_addr)
{
	owner.jenv->SetLongField(jmodel_global_ref,owner.jdevs_cpp_peer,(jlong)obj_addr);
}

void JavaDevs::clear_peer()
{
	owner.jenv->SetLongField(jmodel_global_ref,owner.jdevs_cpp_peer,(jlong)NULL);
	owner.jenv->DeleteGlobalRef(jmodel_global_ref);
	jmodel_global_ref = NULL;
}

JavaDevs::~JavaDevs()
{
	if (jmodel_global_ref != NULL)
		clear_peer();
}
