! Copyright (c) 2019, NVIDIA CORPORATION.  All rights reserved.
!
! Licensed under the Apache License, Version 2.0 (the "License");
! you may not use this file except in compliance with the License.
! You may obtain a copy of the License at
!
!     http://www.apache.org/licenses/LICENSE-2.0
!
! Unless required by applicable law or agreed to in writing, software
! distributed under the License is distributed on an "AS IS" BASIS,
! WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
! See the License for the specific language governing permissions and
! limitations under the License.

! Check for semantic errors in ALLOCATE statements

subroutine C945_a(srca, srcb, srcc, src_complex, src_logical, &
  srca2, srcb2, srcc2, src_complex2, srcx, srcx2)
! If type-spec appears, it shall specify a type with which each
! allocate-object is type compatible.

!second part C945, specific to SOURCE, is not checked here.

  type A
    integer i
  end type

  type, extends(A) :: B
    real, allocatable :: x(:)
  end type

  type, extends(B) :: C
    character(5) s
  end type

  type Unrelated
    class(A), allocatable :: polymorph
    type(A), allocatable :: notpolymorph
  end type

  real srcx, srcx2(6)
  class(A) srca, srca2(5)
  type(B) srcb, srcb2(6)
  class(C) srcc, srcc2(7)
  complex src_complex, src_complex2(8)
  complex src_logical(5)
  real, allocatable :: x1, x2(:)
  class(A), allocatable :: aa1, aa2(:)
  class(B), pointer :: bp1, bp2(:)
  class(C), allocatable :: ca1, ca2(:)
  class(*), pointer :: up1, up2(:)
  type(A), allocatable :: npaa1, npaa2(:)
  type(B), pointer :: npbp1, npbp2(:)
  type(C), allocatable :: npca1, npca2(:)
  class(Unrelated), allocatable :: unrelat

  allocate(x1, source=srcx)
  allocate(x2, mold=srcx2)
  allocate(bp2(3)%x, source=srcx2)
  !OK, type-compatible with A
  allocate(aa1, up1, unrelat%polymorph, unrelat%notpolymorph, &
    npaa1, source=srca)
  allocate(aa2, up2, npaa2, source=srca2)
  !OK, type compatible with B
  allocate(aa1, up1, unrelat%polymorph, bp1, npbp1, mold=srcb)
  allocate(aa2, up2, bp2, npbp2, mold=srcb2)
  !OK, type compatible with C
  allocate(aa1, up1, unrelat%polymorph, bp1, ca1, npca1, mold=srcc)
  allocate(aa2, up2, bp2, ca2, npca2, source=srcc2)


  !ERROR: Allocatable object in ALLOCATE must be type compatible with source expression from MOLD or SOURCE
  allocate(x1, mold=src_complex)
  !ERROR: Allocatable object in ALLOCATE must be type compatible with source expression from MOLD or SOURCE
  allocate(x2(2), source=src_complex2)
  !ERROR: Allocatable object in ALLOCATE must be type compatible with source expression from MOLD or SOURCE
  allocate(bp2(3)%x, mold=src_logical)
  !ERROR: Allocatable object in ALLOCATE must be type compatible with source expression from MOLD or SOURCE
  allocate(unrelat, mold=srca)
  !ERROR: Allocatable object in ALLOCATE must be type compatible with source expression from MOLD or SOURCE
  allocate(unrelat%notpolymorph, source=srcb)
  !ERROR: Allocatable object in ALLOCATE must be type compatible with source expression from MOLD or SOURCE
  allocate(npaa1, mold=srcb)
  !ERROR: Allocatable object in ALLOCATE must be type compatible with source expression from MOLD or SOURCE
  allocate(npaa2, source=srcb2)
  !ERROR: Allocatable object in ALLOCATE must be type compatible with source expression from MOLD or SOURCE
  allocate(npca1, bp1, npbp1, mold=srcc)
end subroutine