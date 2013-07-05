<?php

/**
 * Buid a add js list page.
 */
function add_js_list_page_form($form, $form_state) {
	// Return confirm page when need to delete codes.
  if (!empty($form_state['values']['codes'])) {
    return add_js_list_page_form_delete_confirm($form, $form_state, array_filter($form_state['values']['codes']));
  }
	// Build the sortable table header.
	  $header = array(
		'id' => array('data' => t('id'), 'field' => 'aj.id', 'sort' => 'desc'),
	    'name' => array('data' => t('Name'), 'field' => 'aj.name'),
		'description' => array('data' => t('Description'), 'field' => 'aj.description'),
		'site_visibility' => array('data' => t('Site'), 'field' => 'aj.site_visibility'),
		'add_js' => array('data' => t('Add JS'), 'field' => 'aj.add_js'),
		'add_block' => array('data' => t('Add Block'), 'field' => 'aj.add_block'),
		'visibility' => array('data' => t('Status'), 'field' => 'aj.page_visibility'),
		'operations' => array('data' => t('Operations')),
	  );
	// Get js code from DB.
	  $codes = add_js_get_code(NULL, $header, 50);

	  $destination = drupal_get_destination();
	  $options = array();
	foreach ($codes as $delta => $code) {
	    $options[$code->id] = array(
			  'id' => $delta,
		      'name' => check_plain($code->name),
			  'description' => check_plain($code->description),
			  'site_visibility' => check_plain($code->site_visibility),
			  'add_js' => $code->add_js ? t('YES') : t('NO'),
		      'add_block' => $code->add_block ? t('YES') : t('NO'),
			  'visibility' => $code->code_visibility ? t('ON') : t('OFF'),
	    );

	    // Build a list of all the accessible operations for the current js code.
	    $operations = array();
	    if (user_access('edit add js')) {
	      $operations['edit'] = array(
	        'title' => t('edit'),
	        'href' => 'admin/config/content/add-js/edit/' . $code->id,
	        'query' => $destination,
	      );
	    }
	    if (user_access('delete add js')) {
	      $operations['delete'] = array(
	        'title' => t('delete'),
	        'href' => 'admin/config/content/add-js/delete/' . $code->id,
	        'query' => $destination,
	      );
	    }
	    if (count($operations) > 1) {
	      // Render an unordered list of operations js code.
	      $options[$code->id]['operations'] = array(
	        'data' => array(
	          '#theme' => 'links__node_operations',
	          '#links' => $operations,
	          '#attributes' => array('class' => array('links', 'inline')),
	        ),
	      );
	    }
	    elseif (!empty($operations)) {
	      // Render the first and only operation as a js code.
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
	  if (user_access('edit add js') || user_access('delete add js')) {
	    $form['codes'] = array(
	      '#type' => 'tableselect',
	      '#header' => $header,
	      '#options' => $options,
	      '#empty' => t('No js codes available.'),
	    );
	  }
	  // Otherwise, use a simple table.
	  else {
	    unset($header['operations']);
	    $form['codes'] = array(
	      '#theme' => 'table',
	      '#header' => $header,
	      '#rows' => $options,
	      '#empty' => t('No js codes available.'),
	    );
	  }
	  if (!empty($options) && user_access('delete add js')) {
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
function add_js_add_form($form, &$form_state, $id = NULL) {
  $form['add_js_add_form'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add JS Code'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

  if (!empty($id)) {
    $defaults = add_js_get_code($id);
    $form['add_js_add_form']['delete'] = array(
      '#type' => 'submit',
      '#value' => t('Delete'),
      '#weight' => 15,
      '#submit' => array('add_js_add_form_delete_submit'),
    );
  }

$form['add_js_add_form']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#size' => 50,
    '#maxlength' => 255,
    '#description' => t('The name of the JS code'),
	'#required' => TRUE,
	'#default_value' => isset($defaults[$id]->name) ? $defaults[$id]->name: '',
  );
$form['add_js_add_form']['description'] = array(
    '#type' => 'textfield',
    '#title' => t('Description'),
    '#size' => 50,
    '#maxlength' => 255,
    '#description' => t('The human readable description of this rule.'),
	'#required' => TRUE,
	'#default_value' => isset($defaults[$id]->description) ? $defaults[$id]->description: '',
  );
$form['add_js_add_form']['js_code'] = array(
  '#type' => 'fieldset',
  '#title' => t('Add javascript code options'),
  '#collapsible' => TRUE,
  '#collapsed' => FALSE,
);
$form['add_js_add_form']['js_code']['add_js'] = array(
    '#type' => 'checkbox',
    '#title' => t('Add JavaScript'),
    '#description' => t('Add javascript to a selected site below'),
    '#default_value' => isset($defaults[$id]->add_js) ? $defaults[$id]->add_js: '',
  );
$form['add_js_add_form']['js_code']['js'] = array(
    '#type' => 'textarea',
    '#title' => t('JS Code'),
    '#description' => t('The actual JavaScript code goes in here. There is no need to insert the &lt;script&gt; tags.'),
    '#required' => FALSE,
	'#default_value' => isset($defaults[$id]->js) ? $defaults[$id]->js: '',
	'#states' => array(
      'visible' => array(
        'input[name="add_js"]' => array('checked' => TRUE),
      ),
    ),
  );

// placement options fieldset
  $form['add_js_add_form']['js_code']['placement'] = array(
    '#type' => 'fieldset',
    '#title' => t('Placement options'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
	'#states' => array(
      'visible' => array(
        'input[name="add_js"]' => array('checked' => TRUE),
      ),
    ),
  );
  $form['add_js_add_form']['js_code']['placement']['position'] = array(
    '#type' => 'select',
    '#title' => 'Position of the javascipt',
    '#description' => t('Where in the HTML will the JavaScript be placed.'),
    '#options' => array(
      'header' => t('Header'),
      'footer' => t('Footer'),
    ),
    '#default_value' => isset($defaults[$id]->position) ? $defaults[$id]->position: t('Header'),
	'#states' => array(
      'visible' => array(
        'input[name="add_js"]' => array('checked' => TRUE),
      ),
    ),
  );
  $form['add_js_add_form']['js_code']['placement']['preprocess'] = array(
    '#type' => 'checkbox',
    '#title' => t('Preprocess JS'),
    '#description' => t('If the JavaScript is preprocessed, and JavaScript aggregation is enabled, the script file will be aggregated. Warning - this means you will require a theme cache clear in order to regenerate new aggregate files.'),
    '#default_value' => isset($defaults[$id]->preprocess) ? $defaults[$id]->preprocess: '',
	'#states' => array(
      'visible' => array(
        'input[name="inline"]' => array('checked' => FALSE),
      ),
    ),
  );
  $form['add_js_add_form']['js_code']['placement']['inline'] = array(
    '#type' => 'checkbox',
    '#title' => t('Inline JS'),
    '#description' => t('The JavaScript file can also be inline on the page.'),
    '#default_value' => isset($defaults[$id]->inline) ? $defaults[$id]->inline: '',
    '#states' => array(
      'visible' => array(
        'input[name="preprocess"]' => array('checked' => FALSE),
      ),
    ),
  );



	$form['add_js_add_form']['block'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add block options'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
  );

	$form['add_js_add_form']['block']['add_block'] = array(
	    '#type' => 'checkbox',
	    '#title' => t('Add block'),
	    '#description' => t('Add a block to a region for the selected site below'),
	    '#default_value' => isset($defaults[$id]->add_block) ? $defaults[$id]->add_block: '',
	  );
	$form['add_js_add_form']['block']['block_content'] = array(
	    '#type' => 'textarea',
	    '#title' => t('Block Content'),
	    '#description' => t('The actual content of the block goes in here.'),
	    '#required' => FALSE,
		'#default_value' => isset($defaults[$id]->block_content) ? $defaults[$id]->block_content: '',
		'#states' => array(
	      'visible' => array(
	        'input[name="add_block"]' => array('checked' => TRUE),
	      ),
	    ),
	  );
	$form['add_js_add_form']['block']['block_weight'] = array(
	    '#type' => 'textfield',
	    '#size' => 26,
	    '#maxlenghth' => 7,
	    '#title' => t('Weight'),
	    '#description' => t('Enter a number (range is –8388608 to 8388607) to set the weight of the block, weight will decide where is the block in the region '),
	    '#default_value' => isset($defaults[$id]->block_weight) ? $defaults[$id]->block_weight: 0,
		'#states' => array(
	      'visible' => array(
	        'input[name="add_block"]' => array('checked' => TRUE),
	      ),
	    ),
	  );

	// Region settings.
	  $theme_default = variable_get('theme_default', 'bartik');
	  foreach (list_themes() as $key => $theme) {
	    // Only display enabled themes
	    if ($theme->status) {

	      $theme_title = $theme->info['name'];
	      if ($key == $theme_default) {
	        $theme_title = t('!theme (default theme)', array('!theme' => $theme_title));
			    
	      	$form['add_js_add_form']['block']['block_region'] = array(
		        '#type' => 'select',
		        '#title' => $theme_title,
		        '#default_value' => isset($defaults[$id]->block_region) ? $defaults[$id]->block_region: NULL,
		        '#empty_value' => BLOCK_REGION_NONE,
				'#description' => t('Specify in which regions this block is displayed.'),
		        '#options' => system_region_list($key, REGIONS_VISIBLE),
		        '#states' => array(
			      'visible' => array(
			        'input[name="add_block"]' => array('checked' => TRUE),
			      ),
			    ),
		      );
			}
	    }
	  }

	//custom for news site
		$form['add_js_add_form']['sites'] = array(
		    '#type' => 'fieldset',
		    '#title' => t('Sites'),
		    '#collapsible' => TRUE,
		    '#collapsed' => FALSE,
		  );
		$site_options = array(
		    'www.auburnjournal.com' => t('www.auburnjournal.com'),
			'www.thepresstribune.com' => t('www.thepresstribune.com'),
			'www.theloomisnews.com' => t('www.theloomisnews.com'),
			'www.colfaxrecord.com' => t('www.colfaxrecord.com'),
			'www.placerherald.com' => t('www.placerherald.com'),
			'www.lincolnnewsmessenger.com' => t('www.lincolnnewsmessenger.com'),
			'www.folsomtelegraph.com' => t('www.folsomtelegraph.com'),
			'www.edhtelegraph.com' => t('www.edhtelegraph.com'),
			'www.placersports.com' => t('www.placersports.com'),
		  );
		$form['add_js_add_form']['sites']['site_visibility'] = array(
		    '#type' => 'radios',
		    '#title' => t('Add js code to specific site'),
		    '#options' => $site_options,
		    '#default_value' => isset($defaults[$id]->site_visibility) ? $defaults[$id]->site_visibility: '',
			'#required' => TRUE,
		  );
		//end custom for news site

 // page visibility fieldset. Code ported from googleanalytics.module
  $form['add_js_add_form']['pages'] = array(
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
  $form['add_js_add_form']['pages']['page_visibility'] = array(
    '#type' => 'radios',
    '#title' => t('Add tracking to specific pages'),
    '#options' => $options,
    '#default_value' => isset($defaults[$id]->page_visibility) ? $defaults[$id]->page_visibility: 1,
  );
  $form['add_js_add_form']['pages']['page_visibility_pages'] = array(
    '#type' => 'textarea',
    '#title' => $title,
    '#title_display' => 'invisible',
    '#default_value' => isset($defaults[$id]->page_visibility_pages) ? $defaults[$id]->page_visibility_pages: '',
    '#description' => $description,
    '#rows' => 10,
  );

$form['add_js_add_form']['code_visibility'] = array(
    '#type' => 'radios',
   	'#title' => t('Enable/Disable'),
    '#description' => t('Enable or Disable the js code and the block.'),
    '#options' => array(
	  1 => t('Enable'),
      0 => t('Disable'),     
    ),
    '#default_value' => isset($defaults[$id]->code_visibility) ? $defaults[$id]->code_visibility : 1,
  );
  
  $form['add_js_add_form']['id'] = array(
    '#type' => 'hidden',
    '#value' => !empty($id) ? $id : NULL,
  );
  $form['add_js_add_form']['save'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 15,
    '#submit' => array('add_js_add_form_save_submit'),
  );

  return $form;
}


/**
 * Submit for save action.
 */
function add_js_add_form_save_submit($form, &$form_state) {
  	$values = array(
	    'name' => trim($form_state['values']['name']),
		'js' => trim($form_state['values']['js']),
		'description' => trim($form_state['values']['description']),
		'add_js'=>$form_state['values']['add_js'],
		'position'=>$form_state['values']['position'],
		'preprocess'=>$form_state['values']['preprocess'],
		'inline'=>$form_state['values']['inline'],
		'site_visibility' => $form_state['values']['site_visibility'],
		'add_block'=>$form_state['values']['add_block'],
		'block_content' => $form_state['values']['block_content'],
		'block_weight' => $form_state['values']['block_weight'],
		'block_region' => $form_state['values']['block_region'],
		'page_visibility' => $form_state['values']['page_visibility'],	 
		'page_visibility_pages' => trim($form_state['values']['page_visibility_pages']),
	    'code_visibility' => $form_state['values']['code_visibility'],
		
	  );
	if (isset($form_state['values']['id'])) {
	    add_js_add_update_code('update', $values, $form_state['values']['id']);
	    drupal_set_message(t('Done'));
	  }
	  else {
	add_js_add_update_code('insert', $values);
	drupal_set_message('JS code successfully added');
	}
$form_state['redirect'] = 'admin/config/content/add-js/list';
}

/**
 * Validate form for add or edit page.
 */
function add_js_add_form_validate($form, &$form_state) {
 	if ($form_state['values']['op'] == t('Save')) {
		$limit = $form_state['values']['block_weight'];
		  if (!is_numeric($limit) || (int) $limit < -8388608 || (int) $limit > 8388607){
		    form_set_error('add_js_add_form][block][block_weight', t('Weight of block must be numeric and in the range of (–8388608,8388607).'));
		  }
		if(empty($form_state['values']['site_visibility'])){
			form_set_error('add_js_add_form][site', t('Please choose a site.'));
		}
	}
}


/**
 * Submit for a Add JS list page.
 */
function add_js_list_page_form_submit($form, &$form_state) {
  $form_state['rebuild'] = TRUE;
}

/**
 * Confirm form for delete operation.
 */
function add_js_list_page_form_delete_confirm($form, &$form_state, $codes) {
  $form['codes']['#tree'] = TRUE;
  foreach ($codes as $cid) {
    $form['codes'][$cid] = array(
      '#type' => 'hidden',
      '#value' => $cid,
    );
  }
  $form['#submit'][] = 'add_js_list_page_form_delete_confirm_submit';
  $confirm_question = format_plural(count($codes), 'Are you sure you want to delete this item?', 'Are you sure you want to delete these items?');
  return confirm_form($form, $confirm_question, 'admin/config/content/add-js/list', t('This action cannot be undone.'), t('Delete'), t('Cancel'));
}

/**
 * Submit for delete operation.
 */
function add_js_list_page_form_delete_confirm_submit($form, &$form_state) {
  foreach ($form_state['values']['codes'] as $cid) {
    if (!empty($cid) && is_numeric($cid)) {
      add_js_del_code($cid);
    }
  }
}

/**
 * Submit for delete action.
 */
function add_js_add_form_delete_submit($form, &$form_state) {
  $destination = array();
  if (isset($_GET['destination'])) {
    $destination = drupal_get_destination();
    unset($_GET['destination']);
  }
  $form_state['redirect'] = array('admin/config/content/add-js/delete/' . $form_state['values']['id'], array('query' => $destination));
}
/**
 * Form builder for delete confirm page.
 */
function add_js_del_form($form, &$form_state, $id) {
  $form['add_js_del_form']['message'] = array(
    '#markup' => t('This action cannot be undone.'),
    '#prefix' => '<div class="form-item">',
    '#suffix' => '</div>',
  );
  $form['add_js_del_form']['delete'] = array(
    '#type' => 'submit',
    '#value' => t('Delete'),
  );
  $form['add_js_del_form']['cancel'] = array(
    '#markup' => l(t('Cancel'), 'admin/config/content/add-js/list'),
  );
  $form['add_js_del_form']['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );

  return $form;
}

/**
 * Submit form for delete confirm page.
 */
function add_js_del_form_submit($form, &$form_state) {
  add_js_del_code($form_state['values']['id']);
  $form_state['redirect'] = 'admin/config/content/add_js/list';
}


