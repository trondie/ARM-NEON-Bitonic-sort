if HAVE_OMPSS
bin_PROGRAMS = merge-sort-ompss

merge_sort_ompss_SOURCES = multisort-ompss-tree.c kernel.c
merge_sort_ompss_LDADD = $(NANOX_LDLIBS)
merge_sort_ompss_LINK = $(MCC) --ompss $(CXXFLAGS) -o $@ $(NANOX_LDFLAGS)

include $(top_srcdir)/mk/rules.ompss.mk
endif
