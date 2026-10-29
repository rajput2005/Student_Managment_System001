/**
 * EduPulse - Student Management System
 * Frontend Application Controller
 * Handles UI interactions, API calls to C++ backend, state, modals, and notifications.
 */

document.addEventListener('DOMContentLoaded', () => {
  // ==========================================
  // App State
  // ==========================================
  const state = {
    students: [],
    stats: null,
    filters: {
      search: '',
      department: 'All',
      status: 'All',
      sortBy: 'id',
      sortOrder: 'DESC'
    },
    activeStudent: null,
    studentToDelete: null,
    isSubmitting: false
  };

  // ==========================================
  // DOM Elements
  // ==========================================
  // Stats
  const elStatTotal = document.getElementById('statTotalStudents');
  const elStatActive = document.getElementById('statActiveStudents');
  const elStatAvgGpa = document.getElementById('statAvgGpa');
  const elStatDepts = document.getElementById('statDepartmentsCount');

  // Search & Filter controls
  const elSearchInput = document.getElementById('searchInput');
  const elClearSearchBtn = document.getElementById('clearSearchBtn');
  const elFilterStatus = document.getElementById('filterStatus');
  const elSortBy = document.getElementById('sortBySelect');
  const elDepartmentPills = document.getElementById('departmentPills');
  const elRefreshBtn = document.getElementById('refreshBtn');
  const elRefreshIcon = document.getElementById('refreshIcon');
  const elExportCsvBtn = document.getElementById('exportCsvBtn');
  const elResetFiltersBtn = document.getElementById('resetFiltersBtn');

  // Table
  const elTableBody = document.getElementById('studentsTableBody');
  const elTableLoading = document.getElementById('tableLoading');
  const elTableEmpty = document.getElementById('tableEmpty');
  const elRecordCount = document.getElementById('recordCountText');

  // Theme Toggle
  const elThemeToggleBtn = document.getElementById('themeToggleBtn');

  // Modals
  const elStudentModal = document.getElementById('studentModal');
  const elStudentForm = document.getElementById('studentForm');
  const elModalTitle = document.getElementById('modalTitle');
  const elModalSubtitle = document.getElementById('modalSubtitle');
  const elCloseStudentModalBtn = document.getElementById('closeStudentModalBtn');
  const elCancelStudentModalBtn = document.getElementById('cancelStudentModalBtn');
  const elSaveStudentBtn = document.getElementById('saveStudentBtn');
  const elOpenAddStudentBtn = document.getElementById('openAddStudentBtn');

  // Form Fields
  const fId = document.getElementById('studentDbId');
  const fStudentId = document.getElementById('formStudentId');
  const fFirstName = document.getElementById('formFirstName');
  const fLastName = document.getElementById('formLastName');
  const fEmail = document.getElementById('formEmail');
  const fPhone = document.getElementById('formPhone');
  const fDob = document.getElementById('formDob');
  const fGender = document.getElementById('formGender');
  const fDepartment = document.getElementById('formDepartment');
  const fYear = document.getElementById('formYear');
  const fGpa = document.getElementById('formGpa');
  const fStatus = document.getElementById('formStatus');
  const fAddress = document.getElementById('formAddress');

  // View Modal
  const elViewModal = document.getElementById('viewModal');
  const elCloseViewModalBtn = document.getElementById('closeViewModalBtn');
  const elViewAvatar = document.getElementById('viewAvatar');
  const elViewFullName = document.getElementById('viewFullName');
  const elViewStudentIdBadge = document.getElementById('viewStudentIdBadge');
  const elViewStatusPill = document.getElementById('viewStatusPill');
  const elViewDepartment = document.getElementById('viewDepartment');
  const elViewYear = document.getElementById('viewYear');
  const elViewGpaBadge = document.getElementById('viewGpaBadge');
  const elViewEmail = document.getElementById('viewEmail');
  const elViewPhone = document.getElementById('viewPhone');
  const elViewDob = document.getElementById('viewDob');
  const elViewGender = document.getElementById('viewGender');
  const elViewCreatedAt = document.getElementById('viewCreatedAt');
  const elViewAddress = document.getElementById('viewAddress');
  const elViewEditBtn = document.getElementById('viewEditBtn');
  const elViewDeleteBtn = document.getElementById('viewDeleteBtn');

  // Delete Modal
  const elDeleteModal = document.getElementById('deleteModal');
  const elCloseDeleteModalBtn = document.getElementById('closeDeleteModalBtn');
  const elCancelDeleteBtn = document.getElementById('cancelDeleteBtn');
  const elConfirmDeleteBtn = document.getElementById('confirmDeleteBtn');
  const elDeleteStudentName = document.getElementById('deleteStudentName');
  const elDeleteStudentRoll = document.getElementById('deleteStudentRoll');

  // Toast Container
  const elToastContainer = document.getElementById('toastContainer');

  // ==========================================
  // Color & Avatar Helpers
  // ==========================================
  const avatarColors = [
    'linear-gradient(135deg, #4f46e5, #7c3aed)',
    'linear-gradient(135deg, #2563eb, #38bdf8)',
    'linear-gradient(135deg, #059669, #10b981)',
    'linear-gradient(135deg, #d97706, #f59e0b)',
    'linear-gradient(135deg, #db2777, #f43f5e)',
    'linear-gradient(135deg, #7c2d12, #ea580c)',
    'linear-gradient(135deg, #4338ca, #6366f1)'
  ];

  function getAvatarGradient(name) {
    let hash = 0;
    for (let i = 0; i < name.length; i++) {
      hash = name.charCodeAt(i) + ((hash << 5) - hash);
    }
    const index = Math.abs(hash) % avatarColors.length;
    return avatarColors[index];
  }

  function getInitials(firstName, lastName) {
    const f = firstName ? firstName.charAt(0) : '';
    const l = lastName ? lastName.charAt(0) : '';
    return (f + l).toUpperCase() || 'ST';
  }

  // ==========================================
  // Toast Notifications
  // ==========================================
  function showToast(message, type = 'success') {
    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;

    let iconSvg = '';
    if (type === 'success') {
      iconSvg = `<svg class="toast-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="20 6 9 17 4 12"></polyline></svg>`;
    } else if (type === 'error') {
      iconSvg = `<svg class="toast-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"></circle><line x1="15" y1="9" x2="9" y2="15"></line><line x1="9" y1="9" x2="15" y2="15"></line></svg>`;
    } else {
      iconSvg = `<svg class="toast-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"></circle><line x1="12" y1="16" x2="12" y2="12"></line><line x1="12" y1="8" x2="12.01" y2="8"></line></svg>`;
    }

    toast.innerHTML = `
      ${iconSvg}
      <span class="toast-message">${escapeHtml(message)}</span>
      <button class="toast-close" aria-label="Dismiss">&times;</button>
    `;

    const closeBtn = toast.querySelector('.toast-close');
    closeBtn.addEventListener('click', () => dismissToast(toast));

    elToastContainer.appendChild(toast);

    // Auto dismiss after 4 seconds
    setTimeout(() => {
      dismissToast(toast);
    }, 4000);
  }

  function dismissToast(toast) {
    if (!toast || toast.classList.contains('toast-hiding')) return;
    toast.classList.add('toast-hiding');
    setTimeout(() => {
      if (toast.parentNode) {
        toast.parentNode.removeChild(toast);
      }
    }, 200);
  }

  function escapeHtml(str) {
    if (!str) return '';
    const div = document.createElement('div');
    div.textContent = str;
    return div.innerHTML;
  }

  // ==========================================
  // API Calls
  // ==========================================
  async function loadData() {
    elTableLoading.style.display = 'flex';
    elTableEmpty.style.display = 'none';
    if (elRefreshIcon) elRefreshIcon.classList.add('spinning');

    try {
      await Promise.all([fetchStudents(), fetchStats()]);
      updateServerStatus(true);
    } catch (err) {
      console.error('Data load error:', err);
      updateServerStatus(false);
      showToast('Could not connect to backend server. Make sure C++ server is running on port 8080.', 'error');
    } finally {
      elTableLoading.style.display = 'none';
      if (elRefreshIcon) elRefreshIcon.classList.remove('spinning');
    }
  }

  async function fetchStudents() {
    const params = new URLSearchParams();
    if (state.filters.search) params.append('search', state.filters.search);
    if (state.filters.department && state.filters.department !== 'All') {
      params.append('department', state.filters.department);
    }
    if (state.filters.status && state.filters.status !== 'All') {
      params.append('status', state.filters.status);
    }
    if (state.filters.sortBy) params.append('sortBy', state.filters.sortBy);
    if (state.filters.sortOrder) params.append('sortOrder', state.filters.sortOrder);

    const res = await fetch(`/api/students?${params.toString()}`);
    if (!res.ok) throw new Error(`HTTP error ${res.status}`);
    const json = await res.json();

    if (json.success && Array.isArray(json.data)) {
      state.students = json.data;
      renderTable();
    } else {
      throw new Error(json.error || 'Failed to load students');
    }
  }

  async function fetchStats() {
    try {
      const res = await fetch('/api/stats');
      if (!res.ok) return;
      const json = await res.json();
      if (json.success && json.data) {
        state.stats = json.data;
        renderStats();
      }
    } catch (e) {
      console.warn('Stats fetch issue:', e);
    }
  }

  function updateServerStatus(online) {
    const badge = document.getElementById('serverStatusBadge');
    if (!badge) return;
    const dot = badge.querySelector('.status-dot');
    const label = badge.querySelector('.status-label');
    if (online) {
      dot.style.backgroundColor = 'var(--emerald)';
      label.textContent = 'Backend Connected (localhost:8080)';
    } else {
      dot.style.backgroundColor = 'var(--rose)';
      label.textContent = 'Backend Offline';
    }
  }

  // ==========================================
  // Renderers
  // ==========================================
  function renderStats() {
    if (!state.stats) return;
    const s = state.stats;

    elStatTotal.textContent = s.total_students.toLocaleString();
    elStatActive.textContent = s.active_students.toLocaleString();
    elStatAvgGpa.textContent = s.average_gpa > 0 ? s.average_gpa.toFixed(2) : '0.00';

    const deptsCount = s.departments ? Object.keys(s.departments).length : 0;
    elStatDepts.textContent = deptsCount;
  }

  function renderTable() {
    elTableBody.innerHTML = '';
    const students = state.students;

    elRecordCount.textContent = `Showing ${students.length} ${students.length === 1 ? 'student' : 'students'}`;

    if (students.length === 0) {
      elTableEmpty.style.display = 'flex';
      return;
    }

    elTableEmpty.style.display = 'none';

    students.forEach(st => {
      const tr = document.createElement('tr');

      // Initials & gradient
      const fullName = `${st.first_name} ${st.last_name}`;
      const initials = getInitials(st.first_name, st.last_name);
      const gradient = getAvatarGradient(fullName);

      // GPA badge class
      let gpaClass = 'mid';
      if (st.gpa >= 3.7) gpaClass = 'high';
      else if (st.gpa < 3.0) gpaClass = 'low';

      // Status pill class
      const statusLower = (st.status || 'Active').toLowerCase();
      const statusClass = `status-${statusLower}`;

      tr.innerHTML = `
        <td>
          <div class="student-cell">
            <div class="avatar" style="background: ${gradient}">${initials}</div>
            <div class="student-meta">
              <span class="student-name">${escapeHtml(fullName)}</span>
              <span class="student-email">${escapeHtml(st.email)}</span>
            </div>
          </div>
        </td>
        <td>
          <span class="roll-code">${escapeHtml(st.student_id)}</span>
        </td>
        <td>
          <strong>${escapeHtml(st.department)}</strong>
        </td>
        <td>
          <span>${escapeHtml(st.year || '1st Year')}</span>
        </td>
        <td>
          <span class="gpa-badge ${gpaClass}">${st.gpa ? st.gpa.toFixed(2) : '0.00'}</span>
        </td>
        <td>
          <span class="status-pill ${statusClass}">${escapeHtml(st.status || 'Active')}</span>
        </td>
        <td>
          <span style="font-size: 0.85rem; color: var(--text-secondary);">${escapeHtml(st.phone || 'N/A')}</span>
        </td>
        <td class="text-right">
          <div class="action-buttons">
            <button class="action-btn view-btn" title="View Student Profile" data-id="${st.id}">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle></svg>
            </button>
            <button class="action-btn edit-btn" title="Edit Student Record" data-id="${st.id}">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"></path><path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"></path></svg>
            </button>
            <button class="action-btn delete-btn" title="Delete Student" data-id="${st.id}">
              <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"></polyline><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"></path><line x1="10" y1="11" x2="10" y2="17"></line><line x1="14" y1="11" x2="14" y2="17"></line></svg>
            </button>
          </div>
        </td>
      `;

      // Event listeners for action buttons
      tr.querySelector('.view-btn').addEventListener('click', () => openViewModal(st));
      tr.querySelector('.edit-btn').addEventListener('click', () => openEditModal(st));
      tr.querySelector('.delete-btn').addEventListener('click', () => openDeleteModal(st));

      elTableBody.appendChild(tr);
    });
  }

  // ==========================================
  // Modal Handlers
  // ==========================================
  function clearFormErrors() {
    document.querySelectorAll('.form-error').forEach(el => el.textContent = '');
  }

  function openAddModal() {
    clearFormErrors();
    elStudentForm.reset();
    fId.value = '';

    // Generate suggested Student ID based on current year & count
    const year = new Date().getFullYear();
    const count = (state.stats ? state.stats.total_students : state.students.length) + 1;
    fStudentId.value = `STU-${year}-${String(count).padStart(3, '0')}`;
    fStatus.value = 'Active';
    fYear.value = '1st Year';
    fGpa.value = '3.50';

    elModalTitle.textContent = 'Add New Student';
    elModalSubtitle.textContent = 'Enter student academic, personal and contact information';
    elSaveStudentBtn.querySelector('.btn-text').textContent = 'Add Student';

    elStudentModal.style.display = 'flex';
    fStudentId.focus();
  }

  function openEditModal(st) {
    clearFormErrors();
    fId.value = st.id;
    fStudentId.value = st.student_id || '';
    fFirstName.value = st.first_name || '';
    fLastName.value = st.last_name || '';
    fEmail.value = st.email || '';
    fPhone.value = st.phone || '';
    fDob.value = st.dob || '';
    fGender.value = st.gender || 'Prefer not to say';
    fDepartment.value = st.department || '';
    fYear.value = st.year || '1st Year';
    fGpa.value = st.gpa !== undefined ? st.gpa : '';
    fStatus.value = st.status || 'Active';
    fAddress.value = st.address || '';

    elModalTitle.textContent = 'Edit Student Profile';
    elModalSubtitle.textContent = `Updating credentials for ${st.first_name} ${st.last_name}`;
    elSaveStudentBtn.querySelector('.btn-text').textContent = 'Update Changes';

    // Close view modal if it was open
    elViewModal.style.display = 'none';
    elStudentModal.style.display = 'flex';
  }

  function closeStudentModal() {
    elStudentModal.style.display = 'none';
  }

  function openViewModal(st) {
    state.activeStudent = st;
    const fullName = `${st.first_name} ${st.last_name}`;
    elViewFullName.textContent = fullName;
    elViewAvatar.textContent = getInitials(st.first_name, st.last_name);
    elViewAvatar.style.background = getAvatarGradient(fullName);
    elViewStudentIdBadge.textContent = st.student_id;

    // Status pill
    const statusLower = (st.status || 'Active').toLowerCase();
    elViewStatusPill.className = `status-pill status-${statusLower}`;
    elViewStatusPill.textContent = st.status || 'Active';

    elViewDepartment.textContent = st.department || '-';
    elViewYear.textContent = st.year || '-';

    // GPA
    const gpa = st.gpa || 0.0;
    elViewGpaBadge.textContent = gpa.toFixed(2);
    elViewGpaBadge.className = 'gpa-badge lg ' + (gpa >= 3.7 ? 'high' : (gpa < 3.0 ? 'low' : 'mid'));

    elViewEmail.textContent = st.email || '-';
    elViewPhone.textContent = st.phone || '-';
    elViewDob.textContent = st.dob || 'Not provided';
    elViewGender.textContent = st.gender || 'Not specified';
    elViewCreatedAt.textContent = st.created_at || 'Recently';
    elViewAddress.textContent = st.address || 'No campus housing or address on file.';

    elViewModal.style.display = 'flex';
  }

  function closeViewModal() {
    elViewModal.style.display = 'none';
    state.activeStudent = null;
  }

  function openDeleteModal(st) {
    state.studentToDelete = st;
    elDeleteStudentName.textContent = `${st.first_name} ${st.last_name}`;
    elDeleteStudentRoll.textContent = st.student_id;
    elDeleteModal.style.display = 'flex';
  }

  function closeDeleteModal() {
    elDeleteModal.style.display = 'none';
    state.studentToDelete = null;
  }

  // ==========================================
  // Form Submission (Create / Update)
  // ==========================================
  elStudentForm.addEventListener('submit', async (e) => {
    e.preventDefault();
    clearFormErrors();

    // Client-side Validation
    let hasError = false;
    if (!fStudentId.value.trim()) {
      document.getElementById('errStudentId').textContent = 'Student ID is required.';
      hasError = true;
    }
    if (!fFirstName.value.trim()) {
      document.getElementById('errFirstName').textContent = 'First name is required.';
      hasError = true;
    }
    if (!fLastName.value.trim()) {
      document.getElementById('errLastName').textContent = 'Last name is required.';
      hasError = true;
    }
    if (!fEmail.value.trim()) {
      document.getElementById('errEmail').textContent = 'Email address is required.';
      hasError = true;
    } else if (!/^[^\s@]+@[^\s@]+\.[^\s@]+$/.test(fEmail.value.trim())) {
      document.getElementById('errEmail').textContent = 'Please enter a valid email address.';
      hasError = true;
    }
    if (!fDepartment.value) {
      document.getElementById('errDepartment').textContent = 'Please select an academic department.';
      hasError = true;
    }

    const gpaVal = parseFloat(fGpa.value);
    if (isNaN(gpaVal) || gpaVal < 0 || gpaVal > 4.0) {
      document.getElementById('errGpa').textContent = 'GPA must be between 0.00 and 4.00.';
      hasError = true;
    }

    if (hasError) return;

    const studentData = {
      student_id: fStudentId.value.trim(),
      first_name: fFirstName.value.trim(),
      last_name: fLastName.value.trim(),
      email: fEmail.value.trim(),
      phone: fPhone.value.trim(),
      dob: fDob.value,
      gender: fGender.value,
      department: fDepartment.value,
      year: fYear.value,
      gpa: isNaN(gpaVal) ? 0.0 : gpaVal,
      status: fStatus.value,
      address: fAddress.value.trim()
    };

    const isEdit = Boolean(fId.value);
    const url = isEdit ? `/api/students/${fId.value}` : '/api/students';
    const method = isEdit ? 'PUT' : 'POST';

    // Spinner
    const btnSpinner = elSaveStudentBtn.querySelector('.btn-spinner');
    const btnText = elSaveStudentBtn.querySelector('.btn-text');
    btnSpinner.style.display = 'inline-block';
    elSaveStudentBtn.disabled = true;

    try {
      const res = await fetch(url, {
        method: method,
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(studentData)
      });

      const result = await res.json();
      if (!res.ok || !result.success) {
        throw new Error(result.error || 'Server rejected request');
      }

      closeStudentModal();
      showToast(result.message || (isEdit ? 'Student updated successfully!' : 'Student added successfully!'), 'success');
      await loadData();
    } catch (err) {
      console.error('Save student error:', err);
      showToast(err.message, 'error');
    } finally {
      btnSpinner.style.display = 'none';
      elSaveStudentBtn.disabled = false;
    }
  });

  // ==========================================
  // Delete Student Action
  // ==========================================
  elConfirmDeleteBtn.addEventListener('click', async () => {
    if (!state.studentToDelete) return;
    const id = state.studentToDelete.id;
    const name = `${state.studentToDelete.first_name} ${state.studentToDelete.last_name}`;

    elConfirmDeleteBtn.disabled = true;
    try {
      const res = await fetch(`/api/students/${id}`, { method: 'DELETE' });
      const result = await res.json();

      if (!res.ok || !result.success) {
        throw new Error(result.error || 'Failed to delete record');
      }

      closeDeleteModal();
      if (elViewModal.style.display === 'flex') closeViewModal();
      showToast(`Student record for "${name}" has been deleted.`, 'info');
      await loadData();
    } catch (err) {
      console.error('Delete error:', err);
      showToast(err.message, 'error');
    } finally {
      elConfirmDeleteBtn.disabled = false;
    }
  });

  // ==========================================
  // Filter & Search Handlers
  // ==========================================
  let debounceTimeout = null;
  elSearchInput.addEventListener('input', (e) => {
    const val = e.target.value.trim();
    elClearSearchBtn.style.display = val ? 'block' : 'none';

    clearTimeout(debounceTimeout);
    debounceTimeout = setTimeout(() => {
      state.filters.search = val;
      fetchStudents();
    }, 250);
  });

  elClearSearchBtn.addEventListener('click', () => {
    elSearchInput.value = '';
    elClearSearchBtn.style.display = 'none';
    state.filters.search = '';
    fetchStudents();
  });

  elFilterStatus.addEventListener('change', (e) => {
    state.filters.status = e.target.value;
    fetchStudents();
  });

  elSortBy.addEventListener('change', (e) => {
    const [col, dir] = e.target.value.split('_');
    state.filters.sortBy = col;
    state.filters.sortOrder = dir;
    fetchStudents();
  });

  // Department quick filter pills
  elDepartmentPills.addEventListener('click', (e) => {
    const pill = e.target.closest('.pill-btn');
    if (!pill) return;

    elDepartmentPills.querySelectorAll('.pill-btn').forEach(b => b.classList.remove('active'));
    pill.classList.add('active');

    state.filters.department = pill.dataset.dept;
    fetchStudents();
  });

  elResetFiltersBtn.addEventListener('click', () => {
    elSearchInput.value = '';
    elClearSearchBtn.style.display = 'none';
    elFilterStatus.value = 'All';
    elSortBy.value = 'id_DESC';

    elDepartmentPills.querySelectorAll('.pill-btn').forEach(b => b.classList.remove('active'));
    elDepartmentPills.querySelector('[data-dept="All"]').classList.add('active');

    state.filters.search = '';
    state.filters.department = 'All';
    state.filters.status = 'All';
    state.filters.sortBy = 'id';
    state.filters.sortOrder = 'DESC';

    fetchStudents();
  });

  elRefreshBtn.addEventListener('click', () => {
    loadData();
    showToast('Student records refreshed.', 'info');
  });

  // ==========================================
  // Export CSV
  // ==========================================
  elExportCsvBtn.addEventListener('click', () => {
    if (state.students.length === 0) {
      showToast('No student data to export.', 'error');
      return;
    }

    const headers = ['ID', 'Student ID', 'First Name', 'Last Name', 'Email', 'Phone', 'Date of Birth', 'Gender', 'Department', 'Academic Year', 'GPA', 'Status', 'Address', 'Registered Date'];
    const rows = state.students.map(s => [
      s.id,
      `"${(s.student_id || '').replace(/"/g, '""')}"`,
      `"${(s.first_name || '').replace(/"/g, '""')}"`,
      `"${(s.last_name || '').replace(/"/g, '""')}"`,
      `"${(s.email || '').replace(/"/g, '""')}"`,
      `"${(s.phone || '').replace(/"/g, '""')}"`,
      s.dob || '',
      s.gender || '',
      `"${(s.department || '').replace(/"/g, '""')}"`,
      s.year || '',
      s.gpa || 0.0,
      s.status || 'Active',
      `"${(s.address || '').replace(/"/g, '""')}"`,
      `"${(s.created_at || '').replace(/"/g, '""')}"`
    ]);

    const csvContent = [headers.join(','), ...rows.map(r => r.join(','))].join('\r\n');
    const blob = new Blob([csvContent], { type: 'text/csv;charset=utf-8;' });
    const url = URL.createObjectURL(blob);
    const link = document.createElement('a');
    link.setAttribute('href', url);
    link.setAttribute('download', `students_export_${new Date().toISOString().slice(0, 10)}.csv`);
    document.body.appendChild(link);
    link.click();
    document.body.removeChild(link);
    URL.revokeObjectURL(url);

    showToast(`Exported ${state.students.length} student records to CSV!`, 'success');
  });

  // ==========================================
  // View Modal Shortcuts
  // ==========================================
  elViewEditBtn.addEventListener('click', () => {
    if (state.activeStudent) {
      openEditModal(state.activeStudent);
    }
  });

  elViewDeleteBtn.addEventListener('click', () => {
    if (state.activeStudent) {
      openDeleteModal(state.activeStudent);
    }
  });

  // Modal open/close listeners
  elOpenAddStudentBtn.addEventListener('click', openAddModal);
  elCloseStudentModalBtn.addEventListener('click', closeStudentModal);
  elCancelStudentModalBtn.addEventListener('click', closeStudentModal);

  elCloseViewModalBtn.addEventListener('click', closeViewModal);
  elCloseDeleteModalBtn.addEventListener('click', closeDeleteModal);
  elCancelDeleteBtn.addEventListener('click', closeDeleteModal);

  // Close modals on clicking backdrop
  [elStudentModal, elViewModal, elDeleteModal].forEach(modal => {
    modal.addEventListener('click', (e) => {
      if (e.target === modal) {
        modal.style.display = 'none';
      }
    });
  });

  // Close modals on Escape key
  document.addEventListener('keydown', (e) => {
    if (e.key === 'Escape') {
      elStudentModal.style.display = 'none';
      elViewModal.style.display = 'none';
      elDeleteModal.style.display = 'none';
    }
  });

  // ==========================================
  // Theme Toggle (Dark / Light Mode)
  // ==========================================
  function initTheme() {
    const savedTheme = localStorage.getItem('edupulse_theme');
    if (savedTheme) {
      document.documentElement.setAttribute('data-theme', savedTheme);
    } else if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
      document.documentElement.setAttribute('data-theme', 'dark');
    }
  }

  elThemeToggleBtn.addEventListener('click', () => {
    const currentTheme = document.documentElement.getAttribute('data-theme');
    const newTheme = currentTheme === 'dark' ? 'light' : 'dark';
    document.documentElement.setAttribute('data-theme', newTheme);
    localStorage.setItem('edupulse_theme', newTheme);
  });

  // ==========================================
  // Initialization
  // ==========================================
  initTheme();
  loadData();
});
