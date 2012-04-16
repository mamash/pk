init() {
  cmd=$1
  case ${cmd} in
    bootstrap) ;;
    build) ;;
    changelog) ;;
    check) ;;
    clean) ;;
    fix) ;;
    help) ;;
    info) ;;
    init) ;;
    licenses) ;;
    mount) ;;
    prune) ;;
    sync) ;;
    *) cmd=help ;;
  esac
  
  shift 1 || true
  
  [ -f ~/.pkrc ] && source ~/.pkrc
  
  : ${pkgsrcdir:=$(defaults pkgsrcdir)}
  : ${abi:=$(defaults abi)}
  : ${compiler:=$(defaults compiler)}
  : ${repo_dir:=$(defaults repo_dir)}
  : ${prefix:=$(defaults prefix)}
  : ${release:=$(defaults release)}
  : ${build_base:=$(defaults build_base)}
  : ${build_dir:=$(defaults build_dir)}
  : ${overlay:=$(defaults overlay)}
  : ${package_lists:=$(defaults package_lists)}
  : ${pkg_dir:=$(defaults pkg_dir)}
  : ${repo_hostname:=$(defaults repo_hostname)}
  : ${repo_user:=$(defaults repo_user)}
  : ${repo_url:=$(defaults repo_url)}
  [ ${#targets[@]} -eq 0 ] && targets=($(defaults targets))
  
  while getopts "a:c:d:fo:p:r:u:" option; do
    case ${option} in
      a) abi=${OPTARG} ;;
      c) compiler=${OPTARG} ;;
      d) repo_dir=${OPTARG} ;;
      f) forced_rebuild=yes ;;
      o) overlay=${OPTARG} ;;
      p) prefix=${OPTARG} ;;
      r) release=${OPTARG} ;;
      u) repo_url=${OPTARG} ;;
      *) echo "Unknown option passed." ;;
    esac
  done
  
  shift $(($OPTIND - 1))
  if [ $# -gt 0 ]; then
    targets=($*)
  fi
  check_compiler
  export PATH=${compiler_dir}:${PATH}:${prefix}/bin:${prefix}/sbin
  abi=$(eval echo ${abi})
  [ ${abi} ] || abi=32
  zfs_base=zones/$(zonename)/base
  /usr/sbin/zfs get name ${zfs_base} >/dev/null 2>&1 && zfs_control=yes || true
  platform=$([[ $(uname -v) =~ ^joyent ]] && echo sdc6 || echo sdc5)
  construct_build_id
  repo_dir=$(eval echo ${repo_dir})
  repo_url=$(eval echo ${repo_url})
  pkg_dir=$(eval echo ${pkg_dir})
  package_lists=$(eval echo ${package_lists})
  build_dir=$(eval echo ${build_dir})
  case ${cmd} in
    build|init)
      log_dir=${build_dir}/$(now)_${cmd}_${build_id}
      log_file=${log_dir}/${script_name}.log
      [ ! -d ${log_dir} ] && mkdir -p ${log_dir}
    ;;
  esac
  [ -d "${overlay}" ] || unset ${overlay}

  if [ ! ${pkgsrcdir} ] || [ ! -d ${pkgsrcdir} ]; then
    log "no pkgsrc tree found. make sure it's available at ${pkgsrcdir} or a path set with 'pkgsrcdir'"
    exit 1
  fi
}

defaults() {
  local var=$1
  case ${var} in
    abi)        echo '$(cd ${pkgsrcdir}/devel/bmake; bmake show-var VARNAME=ABI)';;
    compiler)   echo 'gcc';;
    prefix)     echo '/opt/local';;
    release)    echo "$(cat ${pkgsrcdir}/.git/HEAD|sed 's/^.*[_-]//')";;
    targets)    echo 'common php=52 php=53 ruby=18 ruby=192 ruby=193 python=27 python=31';;
    build_base) echo "${pk_dir}";;
    build_dir)  echo '${build_base}/logs';;
    overlay)    echo "${pk_dir}";;
    package_lists) echo '${overlay}/lists';;
    pkgsrcdir)  echo '/opt/pkgsrc';;
    pkg_dir)    echo '${build_base}/packages/${release}/${build_id}';;
    repo_hostname) echo 'pkgsrc.joyent.com';;
    repo_user)  echo 'pkgsrc';;
    repo_url)   echo 'http://${repo_hostname}/${repo_dir}';;
    repo_dir)   echo '${release}/${build_id}';;
  esac  
}

construct_build_id() {
  # ABI (when non-default)
  [ ${abi} == $(eval echo $(defaults abi)) ] || non_def_abi=yes
  # prefix (when non-default, but generate the escaped string anyway)
  [ ${prefix} == $(defaults prefix) ] || non_def_prefix=yes
  prefix_string=${prefix//\//_}
  prefix_string=${prefix_string#_}

  build_id=${compiler_id}_${abi}${non_def_prefix+_${prefix_string}}
  [ "${build_id}" == "${compiler_id}" ] || non_def_build=yes
  if [ "${zfs_control}" == "yes" ]; then
    zfs_path=${zfs_base}/${prefix_string}/${release}_${abi}
  else
    zfs_path='n/a'
  fi
}

check_compiler() {
  case ${compiler} in
  */*)
    if [ -d ${compiler} ]; then # it's a path
      compiler_dir=$(cd ${compiler} && pwd)
      if [ -d ${compiler_dir}/bin ]; then # it's a parent dir
        compiler_dir=${compiler_dir}/bin
      fi
    elif [ -x ${compiler} ]; then # it's executable
      compiler_dir=$(cd $(dirname ${compiler}) && pwd)
      compiler_cmd=${compiler_dir}/$(basename ${compiler})
    fi
    ;;
  *)
    case ${compiler} in
      studio|CC) compiler=sunpro; compiler_pattern=CC ;;
    esac
    compiler_cmd=$(type -p ${compiler_pattern-${compiler}} || true)
    if [ ${compiler_cmd} ] && [ -x ${compiler_cmd} ]; then # found
      compiler_dir=$(dirname ${compiler_cmd})
    fi
    ;;
  esac

  if [ ! ${compiler_cmd} ]; then # only have dir
    if [ ! ${compiler_dir} ]; then
      log "!!! compiler ${compiler} unknown or not found"
    else
      if [ -x ${compiler_dir}/CC ]; then # Studio found
        compiler=sunpro
        compiler_cmd=${compiler_dir}/cc
      elif [ -x ${compiler_dir}/gcc ]; then # GCC found
        compiler=gcc
        compiler_cmd=${compiler_dir}/gcc
      else
        log "!!! No known compiler found in ${compiler_dir}"
      fi
    fi
  fi

  # Identify compiler & version

  if ${compiler_cmd} -V 2>&1|grep 'Sun C' >/dev/null; then # Studio
    compiler=sunpro
    case $(${compiler_cmd} -V 2>&1|awk '/Sun C/{print $4}') in
      5.8)  compiler_version=11.0 ;;
      5.9)  compiler_version=12.0 ;;
      5.10) compiler_version=12.1 ;;
      5.11) compiler_version=12.2 ;;
      5.12) compiler_version=12.3 ;;
      *)    compiler_version=0 ;;
    esac
    compiler_id=${compiler}${compiler_version%%\.*}
  elif ${compiler_cmd} -v 2>&1|grep 'gcc version' >/dev/null; then # GCC
    compiler=gcc
    compiler_version=$(${compiler_cmd} -dumpversion 2>/dev/null || echo 0)
    compiler_id=${compiler}$(awk -F\. '{print $1 $2}' <<<${compiler_version})
  else
    compiler=unknown
    compiler_version=0
    compiler_id=unknown
  fi

}

construct_bmake_env() {
  bmake_env=(${bmake_env[@]} ABI="${abi}")
  bmake_env=(${bmake_env[@]} PKGSRC_COMPILER="${compiler}")
  case ${compiler} in
    sunpro)
      bmake_env=(${bmake_env[@]} SUNWSPROBASE="${compiler_dir%/bin}") ;;
  esac
  if [ ${version_string} ]; then
    bmake_env=(${bmake_env[@]} ${version_string})
  fi
  bmake_env=(${bmake_env[@]} LOCALBASE="${prefix}")
  return 0
}

zfs() {
  if [ "${zfs_control}" == "yes" ]; then
    /usr/sbin/zfs $@
  else
    if [ ! ${zfs_warned} ]; then
      log "skipping 'zfs ${1}' functionality, ZFS not delegated properly"
      zfs_warned=yes
    fi
  fi
}

create_prefix() {
  if ! zfs list -Ho name -t filesystem ${zfs_base}/${prefix_string} >/dev/null 2>&1; then
    log "creating a ZFS node for ${prefix}"
    zfs create ${zfs_base}/${prefix_string} >/dev/null 2>&1
    zfs set mountpoint=none ${zfs_base}/${prefix_string} >/dev/null 2>&1
  fi

  if ! zfs list -Ho name -t filesystem ${zfs_path} >/dev/null 2>&1; then
    log "creating a ZFS node for ${prefix}/${release}"
    zfs create ${zfs_path} >/dev/null 2>&1
  fi
}

snapshot_prefix() {
  snapshot=${1}
  if [ ! ${snapshot} ]; then
    return 1
  fi
  log "creating a ${snapshot} snapshot"
  zfs snapshot ${zfs_path}@${snapshot}
}

mount_prefix() {
  if [ ! ${prefix} ] || [ ! ${release} ]; then
    return 1
  fi
  for fs in $(zfs get -rHo name,value mountpoint ${zfs_base} |\
    awk '{ if ($2=="'${prefix}'") print $1 }'); do
    log "unmounting the previous prefix"
    zfs set mountpoint=none ${fs}
  done
  log "mounting the new prefix"
  zfs set mountpoint=${prefix} ${zfs_path}
}

cleanup() {
  zfs rollback ${zfs_path}@start
  if [ -d /tmp/pkgsrc ]; then
    rm -rf /tmp/pkgsrc
  fi
}

now() {
  date +%Y%m%d-%H%M%S
}

log() {
  if [ ${log_file} ]; then
    echo "$(now): $@" | tee -a ${log_file}
  else
    echo "$(now): $@"
  fi
}


find_tool() {
  local tool=$1
  local tool_path=$(type -p ${tool})
  if [ $? -gt 0 ] || [ ! -x "${tool_path}" ]; then
    log "couldn't find usable ${tool}"
    return 1
  fi
  eval "${tool}=${tool_path}"
}

list_package_files() {
  /bin/ls -l ${pkg_dir}/All 2>/dev/null|sort -f \
    > ${package_log}.$1
}

diff_package_files() {
  /usr/bin/diff ${package_log}.ante ${package_log}.post | \
    /usr/bin/grep '>' | /usr/bin/awk '{print $10}'
}


needs_fixing() {
  log "checking ${pkgbase}"

  found=0
  for lib in ${bad_libs[@]}; do
    pkg_info -Q REQUIRES ${pkgfile} | grep "^${lib}" >/dev/null && \
      ((found++))
  done
  [ ${found} -gt 0 ] && return 0
  return 1
}

fix_requires() {
  log "fixing ${pkgbase}"

  pkg_add ${pkgfile} >/dev/null 2>&1

  for lib in ${bad_libs[@]}; do
    /usr/bin/sed -e "\,REQUIRES=${lib},d" ${pkgdbdir}/+BUILD_INFO \
      > /tmp/.pk-build_info && \
      cp -p /tmp/.pk-build_info ${pkgdbdir}/+BUILD_INFO
    rm /tmp/.pk-build_info 2>/dev/null
  done
  PKG_DBDIR=${prefix}/pkg ${pkg_tarup} -d $(dirname ${pkgfile}) ${pkgbase} >/dev/null 2>&1 || \
    log "re-packaging failed"
}

changelog_get_versions() {
  local pkg=$1
  local file=$2
  local versions=$(awk -F= '{if($1=="'${pkg}'") print $2}' ${file})
  echo ${versions}
}

find_prev_package_set() {
  local cur_release=${release}
  local cur_build_id=${build_id}
  cur_year=${release%Q*}
  cur_quarter=${release#*Q}
  case ${cur_quarter} in
    1) prev_year=$((cur_year-1)); prev_quarter=4;;
    *) prev_year=${cur_year}; prev_quarter=$((cur_quarter-1));;
  esac
  prev_release=${prev_year}Q${prev_quarter}

  for prev_build_id in ${cur_build_id} ${cur_build_id%[0-9]}? "not-found"; do
    if /bin/ls -d ${build_base}/packages/${prev_release}/${prev_build_id} 2>/dev/null |\
         wc -l | grep -w 1 >/dev/null; then
       prev_build_id=$(basename ${build_base}/packages/${prev_release}/${prev_build_id})
       break
    fi
  done
  
  echo "${prev_release}/${prev_build_id}"
}

generate_changelog() {
  prev_package_set=$1

  if [ ! ${prev_package_set} ]; then
    log "no previous package set provided"
    return 1
  fi

  prev_pkg_dir=${build_base}/packages/${prev_package_set}

  ls -1 ${prev_pkg_dir}/All | grep 'tgz$' | sort -f |\
    nbsed -E 's/\.tgz$//; s/nb[0-9]+$//; s/-([^-]*)$/=\1/' > /tmp/.pk-prev-repo
  ls -1 ${pkg_dir}/All | grep 'tgz$' | sort -f |\
    nbsed -E 's/\.tgz$//; s/nb[0-9]+$//; s/-([^-]*)$/=\1/' > /tmp/.pk-cur-repo
  all_pkgs=($(awk -F= '{print $1}' /tmp/.pk-prev-repo /tmp/.pk-cur-repo|sort -uf))

  for pkg in ${all_pkgs[@]}; do
    in_prev=$(changelog_get_versions ${pkg} /tmp/.pk-prev-repo)
    in_cur=$(changelog_get_versions ${pkg} /tmp/.pk-cur-repo)
    if [ "${in_cur}" ]; then
      if [ ! "${in_prev}" ]; then
        pkg_added=(${pkg_added[@]} ${pkg})
      elif [ "${in_cur}" != "${in_prev}" ]; then
        pkg_changed=(${pkg_changed[@]} ${pkg})
      fi
    else
      pkg_removed=(${pkg_removed[@]} ${pkg})
    fi
  done

  exec 6>&1; exec >${pkg_dir}/CHANGES.txt

  cat <<EOF

Package changelog for ${release}/${build_id}
=====================================================================


New packages
--------------------

EOF
  for pkg in ${pkg_added[@]}; do
    printf "  * %s, %s\n" ${pkg} \
      "$(changelog_get_versions ${pkg} /tmp/.pk-cur-repo|sed 's/ /,/g')"
  done

  cat <<EOF

Packages removed
--------------------

EOF
  for pkg in ${pkg_removed[@]}; do
    printf "  * %s, %s\n" ${pkg} \
      "$(changelog_get_versions ${pkg} /tmp/.pk-prev-repo|sed 's/ /,/g')"
  done  

  cat <<EOF

Packages updated
--------------------

EOF
  for pkg in ${pkg_changed[@]}; do
    printf "  * %s, %s to %s\n" ${pkg} \
      "$(changelog_get_versions ${pkg} /tmp/.pk-prev-repo|sed 's/ /,/g')" \
      "$(changelog_get_versions ${pkg} /tmp/.pk-cur-repo|sed 's/ /,/g')"
  done

  cat <<EOF

Minor packaging changes are ignored in the changelog.
EOF

  exec >&6; exec 6>&-

}
