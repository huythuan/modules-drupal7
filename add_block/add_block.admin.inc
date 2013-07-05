<?php

/**
 * Buid a add block list page.
 */
function add_block_list_page_form($form, $form_state) {
	// Return confirm page when need to delete codes.
  if (!empty($form_state['values']['codes'])) {
    return add_block_list_page_form_delete_confirm($form, $form_state, array_filter($form_state['values']['codes']));
  }
	// Build the sortable table header.
	  $header = array(
		'id' => array('data' => t('id'), 'field' => 'ab.id', 'sort' => 'desc'),
	    'name' => array('data' => t('Name'), 'field' => 'ab.name'),
		'description' => array('data' => t('Description'), 'field' => 'ab.description'),
		'visibility' => array('data' => t('Code ADDED'), 'field' => 'ab.page_visibility'),
		'operations' => array('data' => t('Operations')),
	  );
	// Get block code from DB.
	  $codes = add_block_get_code(NULL, $header, 50);
	  $destination = drupal_get_destination();
	  $options = array();
	foreach ($codes as $delta => $code) {
	    	    $options[$code->id] = array(
			  'id' => $delta,
		      'name' => check_plain($code->name),
			  'description' => check_plain($code->description),
			  'visibility' => $code->code_visibility ? t('ON') : t('OFF'),
	    );

	    // Build a list of all the accessible operations for the current block code.
	    $operations = array();
	    if (user_access('edit add block')) {
	      $operations['edit'] = array(
	        'title' => t('edit'),
	        'href' => 'admin/config/content/add-block/edit/' . $code->id,
	        'query' => $destination,
	      );
	    }
	    if (user_access('delete add block')) {
	      $operations['delete'] = array(
	        'title' => t('delete'),
	        'href' => 'admin/config/content/add-block/delete/' . $code->id,
	        'query' => $destination,
	      );
	    }
	    if (count($operations) > 1) {
	      // Render an unordered list of operations block code.
	      $options[$code->id]['operations'] = array(
	        'data' => array(
	          '#theme' => 'links__node_operations',
	          '#links' => $operations,
	          '#attributes' => array('class' => array('links', 'inline')),
	        ),
	      );
	    }
	    elseif (!empty($operations)) {
	      // Render the first and only operation as a block code.
	      $link = reset($operations);
	      $options[$code->id]['operations'] = array(
	        'data' => array(
	          '#type' => 'link',
	          '#title' => check_plain($link['title']),
	          '#href' => $link['href'],
	          '#options' => array('query' => $link['query']),
	        ),
	      );
	    }
	  }
	
	  // Only use a tableselect when the current user is able to perform any
	  // operations.
	  if (user_access('edit add block') || user_access('delete add block')) {
	    $form['codes'] = array(
	      '#type' => 'tableselect',
	      '#header' => $header,
	      '#options' => $options,
	      '#empty' => t('No block codes available.'),
	    );
	  }
	  // Otherwise, use a simple table.
	  else {
	    unset($header['operations']);
	    $form['codes'] = array(
	      '#theme' => 'table',
	      '#header' => $header,
	      '#rows' => $options,
	      '#empty' => t('No block codes available.'),
	    );
	  }
	  if (!empty($options) && user_access('delete add block')) {
	    $form['submit'] = array(
	      '#type' => 'submit',
	      '#value' => t('Delete selected'),
	    );
	  }
  $form['pager'] = array('#markup' => theme('pager'));
 return $form;
}




/**
 * Form builder for add or edit page.
 */
function add_block_add_form($form, &$form_state, $id = NULL) {
  $form['add_block_add_form'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add Block'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  if (!empty($id)) {
    $defaults = add_block_get_code($id);
    $form['add_block_add_form']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#weight' => 15,
      '#submit' => array('add_block_add_form_delete_submit'),
    );
  }

$form['add_block_add_form']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#size' => 50,
    '#maxlength' => 255,
    '#description' => t('The name of the block code'),
	'#required' => TRUE,
	'#default_value' => isset($defaults[$id]->name) ? $defaults[$id]->name: '',
  );
$form['add_block_add_form']['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#size' => 50,
    '#maxlength' => 255,
    '#description' => t('The human readable description of this rule.'),
	'#required' => TRUE,
	'#default_value' => isset($defaults[$id]->description) ? $defaults[$id]->description: '',
  );
$form['add_block_add_form']['block'] = array(
    '#type' => 'fieldset',
    '#title' => t('Block'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
$form['add_block_add_form']['block']['block_code'] = array(
    '#type' => 'textarea',
    '#title' => t('Code'),
    '#description' => t('The actual block code goes in here.'),
    '#required' => TRUE,
	'#default_value' => isset($defaults[$id]->block) ? $defaults[$id]->block: '',
  );
	// Region settings.
	  $theme_default = variable_get('theme_default', 'bartik');
	  foreach (list_themes() as $key => $theme) {
	    // Only display enabled themes
	    if ($theme->status) {
	      $theme_title = $theme->info['name'];
	      if ($key == $theme_default) {
	        $theme_title = t('!theme (default theme)', array('!theme' => $theme_title)); 
	      	$form['add_block_add_form']['block']['region'] = array(
	        '#type' => 'select',
	        '#title' => $theme_title,
			'#description' => t('Select a region for the block code.'),
	        '#default_value' => isset($defaults[$id]->region) ? $defaults[$id]->region: '',
	        '#empty_value' => BLOCK_REGION_NONE,
	        '#options' => system_region_list($key, REGIONS_VISIBLE),
	      );
			}
	    }
	  }
	

  $form['add_block_add_form']['pages'] = array(
    '#type' => 'fieldset',
    '#title' => t('Pages'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );
  $title = t('Pages');
  $description = t("Specify pages by using their paths. Enter one path per line. The '*' character is a wildcard. Example paths are %blog for the blog page and %blog-wildcard for every personal blog. %front is the front page.", array('%blog' => 'blog', '%blog-wildcard' => 'blog/*', '%front' => '<front>'));
  $options = array(
	1 => t('The listed pages only'),
    0 => t('Every page except the listed pages'),
    
  );
  if (module_exists('php') && user_access('use PHP for tracking visibility')) {
    $options[] = t('Pages on which this PHP code returns <code>TRUE</code> (experts only)');
    $title = t('Pages or PHP code');
    $description .= ' ' . t('If the PHP option is chosen, enter PHP code between %php. Note that executing incorrect PHP code can break your Drupal site.', array('%php' => '<?php ?>'));
  }
  $form['add_block_add_form']['pages']['page_visibility'] = array(
    '#type' => 'radios',
    '#title' => t('Add block to specific pages'),
    '#options' => $options,
    '#default_value' => isset($defaults[$id]->page_visibility) ? $defaults[$id]->page_visibility: 1,
  );
  $form['add_block_add_form']['pages']['page_visibility_pages'] = array(
    '#type' => 'textarea',
    '#title' => $title,
    '#title_display' => 'invisible',
    '#default_value' => isset($defaults[$id]->page_visibility_pages) ? $defaults[$id]->page_visibility_pages: '',
    '#description' => $description,
    '#rows' => 10,
  );

$form['add_block_add_form']['code_visibility'] = array(
    '#type' => 'radios',
   	'#title' => t('Enable/Disable block code'),
    '#options' => array(
	  1 => t('Enable'),
      0 => t('Disable'),     
    ),
    '#default_value' => isset($defaults[$id]->code_visibility) ? $defaults[$id]->code_visibility : 1,
  );
  
  $form['add_block_add_form']['id'] = array(
    '#type' => 'hidden',
    '#value' => !empty($id) ? $id : NULL,
  );
  $form['add_block_add_form']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 15,
    '#submit' => array('add_block_add_form_save_submit'),
  );

  return $form;
}


/**
 * Submit for save action.
 */
function add_block_add_form_save_submit($form, &$form_state) {
  	$values = array(
	    'name' => trim($form_state['values']['name']),
		'block' => trim($form_state['values']['block_code']),
		'description' => trim($form_state['values']['description']),
		'region' => $form_state['values']['region'],
		'page_visibility' => $form_state['values']['page_visibility'],	 
		'page_visibility_pages' => trim($form_state['values']['page_visibility_pages']),
	    'code_visibility' => $form_state['values']['code_visibility'],	    
	  );
	if (isset($form_state['values']['id'])) {
	    add_block_add_update_code('update', $values, $form_state['values']['id']);
	    drupal_set_message(t('Done'));
	  }
	  else {
	add_block_add_update_code('insert', $values);		
	drupal_set_message('block code successfully added');
	}
$form_state['redirect'] = 'admin/config/content/add-block/list';
}



/**
 * Submit for a Add block list page.
 */
function add_block_list_page_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Confirm form for delete operation.
 */
function add_block_list_page_form_delete_confirm($form, &$form_state, $codes) {
  $form['codes']['#tree'] = TRUE;
  foreach ($codes as $cid) {
    $form['codes'][$cid] = array(
      '#type' => 'hidden',
      '#value' => $cid,
    );
  }
  $form['#submit'][] = 'add_block_list_page_form_delete_confirm_submit';
  $confirm_question = format_plural(count($codes), 'Are you sure you want to delete this item?', 'Are you sure you want to delete these items?');
  return confirm_form($form, $confirm_question, 'admin/config/content/add-block/list', t('This action cannot be undone.'), t('Delete'), t('Cancel'));
}

/**
 * Submit for delete operation.
 */
function add_block_list_page_form_delete_confirm_submit($form, &$form_state) {
  foreach ($form_state['values']['codes'] as $cid) {
    if (!empty($cid) && is_numeric($cid)) {
      add_block_del_code($cid);
    }
  }
}

/**
 * Submit for delete action.
 */
function add_block_add_form_delete_submit($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  $form_state['redirect'] = array('admin/config/content/add-block/delete/' . $form_state['values']['id'], array('query' => $destination));
}
/**
 * Form builder for delete confirm page.
 */
function add_block_del_form($form, &$form_state, $id) {
  $form['add_block_del_form']['message'] = array(
    '#markup' => t('This action cannot be undone.'),
    '#prefix' => '<div class="form-item">',
    '#suffix' => '</div>',
  );
  $form['add_block_del_form']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );
  $form['add_block_del_form']['cancel'] = array(
    '#markup' => l(t('Cancel'), 'admin/config/content/add-block/list'),
  );
  $form['add_block_del_form']['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );

  return $form;
}

/**
 * Submit form for delete confirm page.
 */
function add_block_del_form_submit($form, &$form_state) {
  add_block_del_code($form_state['values']['id']);
  $form_state['redirect'] = 'admin/config/content/add_block/list';
}


