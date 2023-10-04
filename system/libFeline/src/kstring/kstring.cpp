#include <feline/kstring.h>

KStringView::operator char const* () const {
	return data;
}

char const* KStringView::get() const {
	return data;
}

size_t KStringView::length() const {
	return len;
}

KStringView::~KStringView() {
}
