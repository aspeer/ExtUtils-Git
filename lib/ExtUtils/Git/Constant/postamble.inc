#  Generic targets that all run the same named method from the ExtUtils::Git module
#
GIT_TARGETS=git_manicheck git_version git_ignore git_tag git_autolicense git_version_increment git_version_increment_commit  \
	git_lint git_version_dump git_version_increment_files git_autocopyright git_remote git_push git_perlver git_kwalitee \
	git_arg


#  Other targets that need the Makefile rebuilt after running
#
GIT_TARGETS_BRANCH=git_branch_master git_branch_development git_master git_development


#  Command to call ExtUtils::Git module with intended target as method
#
GIT_PERL_PM=$(PERL) -M$(EXTUTILS_GIT) \
		-e "$(EXTUTILS_GIT)->$@(@ARGV)" \
		-- $(EXTUTILS_ARGV) $(MAKECMDGOALS)

#  Targets that can run without rebuiling Makefile
#
$(GIT_TARGETS) :
	@$(GIT_PERL_PM)


#  Targets that need makefile rebuilt
$(GIT_TARGETS_BRANCH) : git_ci
	@$(GIT_PERL_PM)
	$(MAKE)


#  Targets that have dependencies, extra steps etc.
#
git_ci : git_manicheck git_autolicense git_autocopyright
	$(GIT_EXE) commit -a



#  Target to build a release
#
git_release : git_status
	$(MAKE) git_version_increment 
	-$(MAKE)
	$(MAKE) git_version_increment_commit 
	#$(GIT_EXE) commit -a -m "VERSION increment"
	$(MAKE) git_tag
	$(MAKE) git_dist



#  Build distribution
#
git_dist : git_status
	$(MAKE) $(DIST_DEFAULT_TARGET)


#  Make new repository and import files into Git
#
git_import : git_init git_autolicense
	@$(GIT_PERL_PM) && \
	$(GIT_EXE) commit -m "MANIFEST Import"


#  Init local git repo
#
git_init :
	@if [ ! -d .git ]; then \
	    $(GIT_EXE) init || exit 1; \
	    $(GIT_EXE) remote add origin $(GIT_REPO)/$(DISTNAME); \
	    $(MAKE) git_ignore
	fi


#  Debug to show args supplied to ExtUtils::Git methods
#
#git_arg :
#	@echo $(EXTUTILS_ARGV)


#  Check all files are in git and up to date
#
git_status : git_manicheck
	@$(GIT_PERL_PM)


#  perl -c -w check of all files
#
cw : all
	@for d in `$(PERL) -e 'print join q[ ], (grep {/\.pm$$/} @ARGV)' -- $(TO_INST_PM)`; \
		do PERL5INC=$(INST_LIB) $(PERL) -I$(INST_LIB) $(MAKEFILELIB) -c -w $$d; done
	@for d in $(EXE_FILES); do PERL5INC=$(INST_LIB) $(PERL) -I$(INST_LIB) $(MAKEFILELIB) -c -w $$d; done


#  perltidy of all files
#
perltidy : all
	@for d in `$(PERL) -e 'print join q[ ], (grep {/\.pm$$/} @ARGV)' -- $(TO_INST_PM)`; \
		do echo $$d; perltidy -se -b $$d; done
	@for d in $(EXE_FILES); do perltidy -x -b $$d; done


#  perlcritic of all files
#
perlcritic : all
	@for d in `$(PERL) -e 'print join q[ ], (grep {/\.pm$$/} @ARGV)' -- $(TO_INST_PM)`; \
		do echo $$d; perlcritic $$d; done
	@for d in $(EXE_FILES); do perlcritic $$d; done


#  Crete changelog
#
git_distchanges : create_distdir
	$(NOECHO) cd $(DISTVNAME) && \
	$(GIT_EXE) rev-list  --all --pretty > $(CHANGELOG_FN) && \
	$(ABSPERLRUN) -MExtUtils::Manifest=maniadd -e 'exit unless -e q{$(CHANGELOG_FN)};' \
		-e 'eval { maniadd({q{$(CHANGELOG_FN)} => q{Module ChangeLog (added by ExtUtils::Git)}}) }' \
		-e '    or print "Could not add $(CHANGELOG_FN) to MANIFEST: $$$${'\''@'\''}\n"' --