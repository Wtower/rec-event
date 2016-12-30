<?php
/**
 * @file
 *   Registration Administration forms file
 * 
 * @author
 *   gkarak@forecasting.gr
 *   
 * @copyright
 *   (c) 2014 George Karakostas
 */

/**
 * Generates the administration form.
 * 
 * Could have used system_settings_form()
 * @see https://api.drupal.org/api/drupal/modules%21system%21system.module/function/system_settings_form/7
 * 
 * Time units:
 * @see http://www.php.net/manual/en/datetime.formats.relative.php
 * 
 * @param array $form
 * @param array $form_state
 * @return array $form
 */
function rec_event_register_admin_form($form, &$form_state) {
  //dpm(field_info_field('field_with_slot_reservations'));
  //dpm(field_info_instance('node', 'field_with_slot_reservations', 'gym_class_group'));
  $form['#tree'] = TRUE;
  // registration times
  $form['reg_start'] = array(
    '#title' => t('Registration opens'),
    '#type' => 'textfield',
    '#default_value' => variable_get('rec_event_register_start', '1 week'),
    '#description' => 
      t('Registration open time related to the event start date time.') . '<br />' . 
      t('Allowed time units: %units.', array('%units' => 
        'sec | second | min | minute | hour | day | week | fortnight | month | year (s)')),
    '#size' => 30,
  );
  $form['reg_end'] = array(
    '#title' => t('Registration closes'),
    '#type' => 'textfield',
    '#default_value' => variable_get('rec_event_register_end', '1 hour'),
    '#description' => 
      t('Registration close time related to the event start date time.') . '<br />' . 
      t('Allowed time units: %units.', array('%units' => 
        'sec | second | min | minute | hour | day | week | fortnight | month | year (s)')),
    '#size' => 30,
  );
  // user reference fields
  $form['active_ref_fields'] = array(
    '#type' => 'fieldset',
    '#title' => t('User reference fields'),
    '#collapsible' => TRUE,
    '#collapsed' => FALSE,
    '#description' => 
      t('A list of all recurring event types with an option to add a user reference field. Only events with this field allow registration.'),
  );
  foreach (rec_event_types() as $event_type_key => $event_type) {
    //if ($event_type->state > -1) { // working state
    $present = !is_null(
        field_info_instance('rec_event', 'event_user_ref', $event_type_key));
    $form['active_ref_fields'][$event_type_key] = array(
      '#type' => 'checkboxes',
      '#title' => entity_label('rec_event_type', $event_type),
      '#options' => ($present)?
        array(1 => t('Field present')): array(1 => t('Add field')),
      '#default_value' => ($present)? array(1 => 1): array(),
      '#disabled' => $present,
      '#description' => ($event_type->state == 0)?
          t('Working calendar'): t('Published calendar'),
    );
  }
  $form['active_ref_fields']['markup'] = array(
    '#markup' =>
      t('<p>Define new types at <a href="@url">Recurring Event Types Administration</a>.</p>',
        array('@url' => '/admin/structure/event-types')),
  );
  /*$form['active_ref_fields']['markup_capacity'] = array(
    '#markup' =>
      t('<p>To allow limited registration capacity, <a href="@url">define</a> a %field field in the node type that is referenced by the event type.</p>',
        array(
          '@url' => '/admin/structure/event-types',
          '%field' => 'field_capacity',
        )),
  );*/
  // Capacity fields
  $form['active_capacity_fields'] = array(
    '#type' => 'fieldset',
    '#title' => t('Event capacity fields'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' =>
      t('A list of all content types with an option to add a capacity field. Only events which reference nodes with this field allow capacity limits.'),
  );
  foreach (node_type_get_types() as $node_type_key => $node_type) {
    $present = !is_null(
      field_info_instance('node', 'field_capacity', $node_type_key));
    $form['active_capacity_fields'][$node_type_key] = array(
      '#type' => 'checkboxes',
      '#title' => $node_type->name,
      '#options' => ($present)?
          array(1 => t('Field present')): array(1 => t('Add field')),
      '#default_value' => ($present)? array(1 => 1): array(),
      '#disabled' => $present,
    );
  }
  $form['active_capacity_fields']['markup'] = array(
    '#markup' =>
      t('<p>Define new types at <a href="@url">Content Types Administration</a>.</p>',
        array('@url' => '/admin/structure/types')),
  );
  // Slot reservation fields
  $form['active_slot_fields'] = array(
    '#type' => 'fieldset',
    '#title' => t('Slot reservation fields'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' =>
      t('A list of all recurring event types with an option to add a field to allow reservations by slot. Only events with this field allow slot registration.'),
  );
  foreach (rec_event_types() as $event_type_key => $event_type) {
    $present = !is_null(
      field_info_instance('rec_event', 'slot_reservation', $event_type_key));
    $form['active_slot_fields'][$event_type_key] = array(
      '#type' => 'checkboxes',
      '#title' => entity_label('rec_event_type', $event_type),
      '#options' => ($present)?
          array(1 => t('Field present')): array(1 => t('Add field')),
      '#default_value' => ($present)? array(1 => 1): array(),
      '#disabled' => $present,
      '#description' => ($event_type->state == 0)?
          t('Working calendar'): t('Published calendar'),
    );
  }
  $form['active_slot_fields']['markup'] = array(
    '#markup' =>
      t('<p>Define new types at <a href="@url">Recurring Event Types Administration</a>.</p>',
        array('@url' => '/admin/structure/event-types')),
  );
  // 'With slot' boolean field
  $form['active_with_slot_fields'] = array(
    '#type' => 'fieldset',
    '#title' => t('Slot reservation specification fields'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
    '#description' =>
      t('A list of all content types with an option to add a slot reservation field. Only events which reference nodes with this field AND with capacity field allow slot registrations.'),
  );
  foreach (node_type_get_types() as $node_type_key => $node_type) {
    $present = !is_null(
      field_info_instance('node', 'field_with_slot_reservations', $node_type_key));
    $form['active_with_slot_fields'][$node_type_key] = array(
      '#type' => 'checkboxes',
      '#title' => $node_type->name,
      '#options' => ($present)?
          array(1 => t('Field present')): array(1 => t('Add field')),
      '#default_value' => ($present)? array(1 => 1): array(),
      '#disabled' => $present,
    );
  }
  $form['active_with_slot_fields']['markup'] = array(
    '#markup' =>
      t('<p>Define new types at <a href="@url">Content Types Administration</a>.</p>',
        array('@url' => '/admin/structure/types')),
  );
  // Image styles
  $form['image_style'] = array(
    '#type' => 'select',
    '#title' => t('Image style for presenting users'),
    '#options' => array_merge(array('none' => t('none')),
      array_combine(array_keys(image_styles()), array_keys(image_styles()))),
    '#default_value' => variable_get('rec_event_register_image_style', ''),
    '#description' => t("Select 'none' to disable user pictures."),
  );
  // Actions
  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save configuration'),
  );
  return $form;
}

/**
 * Validate handler for administration form
 * 
 * Validate opening/closing time intervals
 * 
 * @param array $form
 * @param array $form_state
 */
function rec_event_register_admin_form_validate($form, &$form_state) {
  if (!preg_match('/^(\d+) (sec|second|min|minute|hour|day|week|fortnight|month|year)(|s)$/', 
      $form_state['values']['reg_start'])) 
  {
    form_set_error('reg_start', t('Please enter a valid time unit, eg. %eg.', 
        array('%eg' => '1 week')));
  }
  if (!preg_match('/^(\d+) (sec|second|min|minute|hour|day|week|fortnight|month|year)(|s)$/', 
      $form_state['values']['reg_end'])) 
  {
    form_set_error('reg_end', t('Please enter a valid time unit, eg. %eg.', 
        array('%eg' => '1 week')));
  }
}

/**
 * Submit handler for administration form
 * 
 * @param array $form
 * @param array $form_state
 */
function rec_event_register_admin_form_submit($form, &$form_state) {
  variable_set('rec_event_register_start',
    $form_state['values']['reg_start']);
  variable_set('rec_event_register_end',
    $form_state['values']['reg_end']);
  variable_set('rec_event_register_image_style',
    $form_state['values']['image_style']);
  drupal_set_message(t('The configuration options have been saved.'));
  // loop through event bundles and check for field instances (user ref)
  foreach ($form_state['values']['active_ref_fields'] as 
      $event_type_key => $val) 
  {
    $event_type_label =
      entity_label('rec_event_type', rec_event_type_load($event_type_key));
    if (!empty($val[1]) && 
        !$form['active_ref_fields'][$event_type_key]['#disabled']) 
    {
      rec_event_register_create_field_instances($event_type_key);
      drupal_set_message(t('Created User Reference field instance for type %type.',
          array('%type' => $event_type_label)));
    }
    /*else {
      drupal_set_message(t('Skipped type %type.',
          array('%type' => $event_type_label)));
    }*/
  }
  // loop through node bundles and check for field instances (capacity)
  $node_types = node_type_get_types();
  foreach ($form_state['values']['active_capacity_fields'] as
           $node_type_key => $val)
  {
    $node_type_label = $node_types[$node_type_key]->name;
    if (!empty($val[1]) &&
      !$form['active_capacity_fields'][$node_type_key]['#disabled'])
    {
      rec_event_register_create_field_capacity_instances($node_type_key);
      drupal_set_message(t('Created Capacity field instance for type %type.',
        array('%type' => $node_type_label)));
    }
  }
  // loop through event bundles and check for field instances (slots)
  foreach ($form_state['values']['active_slot_fields'] as
           $event_type_key => $val)
  {
    $event_type_label =
      entity_label('rec_event_type', rec_event_type_load($event_type_key));
    if (!empty($val[1]) &&
      !$form['active_slot_fields'][$event_type_key]['#disabled'])
    {
      rec_event_register_create_field_slot_instances($event_type_key);
      drupal_set_message(t('Created Slot Reservations field instance for type %type.',
        array('%type' => $event_type_label)));
    }
  }
  // loop through node bundles and check for field instances (with slots)
  $node_types = node_type_get_types();
  foreach ($form_state['values']['active_with_slot_fields'] as
           $node_type_key => $val)
  {
    $node_type_label = $node_types[$node_type_key]->name;
    if (!empty($val[1]) &&
      !$form['active_with_slot_fields'][$node_type_key]['#disabled'])
    {
      rec_event_register_create_field_with_slots_instances($node_type_key);
      drupal_set_message(t('Created Slot Reservations Specification field instance for type %type.',
        array('%type' => $node_type_label)));
    }
  }
}