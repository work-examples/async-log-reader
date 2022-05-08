# =================================

MARKDOWNLINT_CONFIG = ./.config/.markdownlint.yaml
YAMLLINT_CONFIG     = ./.config/.yamllint.yaml

# =================================

check-all: check-md check-yaml

# =================================

check-md:
# Using two slashes at the beginning of the paths for Windows bash shell
	docker run --rm --tty --network=none --volume="${CURDIR}:/markdown:ro" \
		--workdir=//markdown/ \
		06kellyjac/markdownlint-cli:0.27.1-alpine \
            --config=${MARKDOWNLINT_CONFIG} \
            --ignore=./gtest/README.md \
			-- ./

check-yaml:
# Using two slashes at the beginning of the paths for Windows bash shell
	docker run --rm --tty --network=none --volume="$(CURDIR):/data:ro" \
		--workdir=//data/ \
		cytopia/yamllint:1.26-0.9 \
            -c=${YAMLLINT_CONFIG} \
			--strict \
			-- ./

# =================================
